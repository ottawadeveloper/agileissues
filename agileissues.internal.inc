<?php

function _agileissues_get_search_vectors() {
  $cached = &drupal_static(__FUNCTION__, array());
  if (empty($cached)) {
    $fromCache = cache_get(__FUNCTION__);
    if (!empty($fromCache->data)) {
      $cached = $fromCache->data;
    }
    if (empty($cached)) {
      $cached = array(
        'story' => _agileissues_build_static_search_vectors('story'),
        'task' => _agileissues_build_static_search_vectors('task'),
      );
      cache_set(__FUNCTION__, $cached);
    }
    // dynamic ones are more sensitive to user-based context, so we use
    // static caching but not DB/memcache.
    _agileissues_build_dynamic_search_vectors($cached['story'], 'story');
    _agileissues_build_dynamic_search_vectors($cached['task'], 'task');
  }
  return $cached;
}

function _agileissues_build_static_search_vectors($for_type) {
  $vectors = module_invoke_all('agile_static_search_vectors', $for_type);
  drupal_alter('agile_static_search_vectors', $vectors, $for_type);
  return $vectors;
}

function _agileissues_build_dynamic_search_vectors(&$existing, $for_type) {
  $existing += module_invoke_all('agile_dynamic_search_vectors', $for_type);
  drupal_alter('agile_dynamic_search_vectors', $existing, $for_type);
}

function _agileissues_get_relationship_definitions() {
  global $language;
  $cached = &drupal_static(__FUNCTION__, array());
  if (empty($cached[$language->language])) {
    $fromCache = cache_get(__FUNCTION__);
    if (!empty($fromCache->data)) {
      $cached = $fromCache->data;
    }
    if (empty($cached[$language->language])) {
      $cached[$language->language] = module_invoke_all('agile_relationships');
      drupal_alter('agile_relationships', $cached[$language->language], $language->language);
      cache_set(__FUNCTION__, $cached);
    }
  }
  return $cached[$language->language];
}

function _agileissues_get_changelog_processors() {
  global $language;
  $cached = &drupal_static(__FUNCTION__, array());
  if (empty($cached[$language->language])) {
    $fromCache = cache_get(__FUNCTION__);
    if (!empty($fromCache->data)) {
      $cached = $fromCache->data;
    }
    if (empty($cached[$language->language])) {
      $cached[$language->language] = array(
        'story' => _agileissues_find_changelog_processors('story'),
        'task' => _agileissues_find_changelog_processors('task'),
      );
      cache_set(__FUNCTION__, $cached);
    }
  }
  return $cached[$language->language];
}

function _agileissues_find_changelog_processors($type) {
  $modules = module_implements('agile_changelog_processors');
  $list = array();
  foreach ($modules as $module) {
    $hook = $module . '_agile_changelog_processors';
    if (function_exists($hook)) {
      $processors = $hook($type);
      foreach ($processors as $key => $def) {
        $list[$key] = $def + array(
          'module' => $module,
        );
      }
    }
  }
  drupal_alter('agile_changelog_processors', $list, $type);
  return $list;
}

function _agileissues_get_metric_definitions() {
  global $language;
  $cached = &drupal_static(__FUNCTION__, array());
  if (empty($cached[$language->language])) {
    $fromCache = cache_get(__FUNCTION__);
    if (!empty($fromCache->data)) {
      $cached = $fromCache->data;
    }
    if (empty($cached[$language->language])) {
      $cached[$language->language] = _agileissues_find_metric_definitions();
      cache_set(__FUNCTION__, $cached);
    }
  }
  return $cached[$language->language];
}

function _agileissues_find_metric_definitions() {
  $modules = module_implements('agile_metrics');
  $list = array();
  foreach ($modules as $module) {
    $hook = $module . '_agile_metrics';
    if (function_exists($hook)) {
      $metrics = $hook();
      foreach ($metrics as $key => $def) {
        $list[$key] = $def + array(
          'module' => $module,
        );
      }
    }
  }
  drupal_alter('agile_metrics', $list, $type);
  return $list;
}

function _agileissues_update_priorities() {
  $lastCheck = variable_get('agileissues_cron_last_check', 0);
  $delay = variable_get('agileissues_cron_delay', 86400);
  if (time() - $lastCheck > $delay) {
    variable_set('agileissues_cron_last_check', time());
    $maxTime = variable_get('agileissues_cron_process_stories', 300);
    $start = time();
    $storyIDs = db_select('agileissues_story', 'ais')
      ->orderBy('updated', 'ASC')
      ->fields('ais', array('id'))
      ->execute()->fetchAllKeyed(0, 0);
    foreach ($storyIDs as $id) {
      if ((time() - $start) > $maxTime) {
        break;
      }
      // resaving updates the priority ID
      $story = agileissues_story_load($id);
      agileissues_story_save($story);
    }
  }
}

function _agileissues_backlog_name($backlog_id) {
  $backlog = agileissues_backlog_load($backlog_id);
  return t($backlog->name);
}

function _agileissues_task_status_name($code) {
  switch ($code) {
    case 'O': return t('Open');
    case 'C': return t('Closed');
  }
  return t('Unknown');
}

function _agileissues_register_changelog($etype, $eid, $property, $old, $new) {
  global $user;
  $change = array(
    'entity_type' => $etype,
    'entity_id' => $eid,
    'property_key' => $property,
    'old_value' => $old,
    'new_value' => $new,
    'created' => time(),
    'uid' => $user->uid,
  );
  db_insert('agileissues_changelog')->fields($change)->execute();
}

function _agileissues_register_all_changes($type, $old, $new) {
  $processors = agileissues_changelog_processors($type);
  foreach ($processors as $key => $processor) {
    if (!empty($processor['include'])) {
      module_load_include($processor['include'], $processor['module']);
    }
    $old_value = NULL;
    $new_value = NULL;
    if (isset($processor['index callback'])) {
      $old_value = $processor['index callback']($old);
      $new_value = $processor['index callback']($new);
    }
    elseif (isset($processor['index'])) {
      $old_value = $old->{$processor['index']};
      $new_value = $new->{$processor['index']};
    }
    if ($old_value != $new_value) {
      _agileissues_register_change($type, $new->id, $key, $old_value, $new_value);
    }
  }
}

function _agileissues_define_relationship($left_type, $left_id, $right_type, $right_id, $relationship_type) {
  $properties = array(
    'relationship_type' => $relationship_type,
    'left_entity_type' => $left_type,
    'left_entity_id' => $left_id,
    'right_entity_type' => $right_type,
    'right_entity_id' => $right_id,
  );
  db_merge('agileissues_relationships')
    ->key($properties)
    ->fields($properties)
    ->execute();
}

function _agileissues_append_actual_notes(&$notes, $type, $object, $view_mode) {
  $ids = db_select('agileissues_notes', 'ain')
    ->condition('entity_type', $type)
    ->condition('entity_id', $object->id)
    ->fields('ain', array('id'))
    ->execute()->fetchAllKeyed(0, 0);
  $entities = agileissues_note_load_multiple($ids);
  $views = entity_view('agile_note', $entities, 'full');
  foreach ($views as $id => $view) {
    $notes[] = $view + array(
      '#agile_date' => $entities[$id]->created,
      '#agile_note_type' => 'agile_note',
    );
  }
}

function _agileissues_append_changelog(&$notes, $type, $object, $view_mode) {
  $change_notes = db_select('agileissues_changelog', 'acl')
    ->fields('acl')
    ->condition('entity_type', $type)
    ->condition('entity_id', $object->id)
    ->execute();
  $processors = agileissues_changelog_processors($type);
  foreach ($change_notes as $note) {
    $processor = $processors[$changenote->property_key];
    $render = array(
      '#theme' => 'agileissues_changelog_entry',
      '#label' => $processor['display'],
      '#old' => $changenote->old_value,
      '#new' => $changenote->new_value,
      '#agile_date' => $changenote->created,
      '#agile_note_type' => 'agile_changelog_note',
      '#user' => $changenote->uid,
    );
    if (!empty($processor['display callback'])) {
      $render['#old'] = $processor['display callback']($render['#old']);
      $render['#new'] = $processor['display callback']($render['#new']);
    }
    $notes[] = $render;
  }
}

function _agileissues_append_tasks(&$notes, $type, $object, $view_mode) {
  $taskIDs = db_select('agileissues_tasks', 'ait')
    ->fields('ait', array('id', 'created'))
    ->condition('story_id', $object->id)
    ->execute()->fetchAll();
  foreach ($taskIDs as $task) {
    $notes[] = array(
      '#theme' => 'agileissues_task_note_entry',
      '#agile_date' => $task->created,
      '#task' => agileissues_task_load($task->id)
    );
  }
}

function _agileissues_save_relationships($object, $obj_type, $relationship_type, $relationship_input) {
  $relationships = explode(",", $relationship_input);
  list($direction, $type) = explode('|', $relationship_type, 2);
  foreach ($relationships as $relationship) {
    list($other_type, $other_id) = explode('|', $relationship, 2);
    if ($direction === 'l') {
      _agileissues_define_relationship($obj_type, $object->id, $other_type, $other_id, $type);
    }
    else {
      _agileissues_define_relationship($other_type, $other_id, $obj_type, $object->id, $type);
    }
  }
}

function _agileissues_clear_relationships($object, $type) {
  db_delete('agileissues_relationships')
    ->condition('left_entity_id', $object->id)
    ->condition('left_entity_type', $type)
    ->execute();
  db_delete('agileissues_relationships')
    ->condition('right_entity_id', $object->id)
    ->condition('right_entity_type', $type)
    ->execute();
}
