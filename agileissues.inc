<?php

function agileissues_construct_changelog_field_processors($name, $field, $instance, $field_property, $display_callback = NULL, $include = 'internal.inc') {
  $languages = array(LANGUAGE_NONE);
  $all = language_list();
  if ($field['translatable']) {
    $languages = array_keys($all);
  }
  $processors = array();
  foreach ($languages as $lang) {
    $processors[$name . '-' . $lang] = array(
      'display' => $instance['label'] . ($lang == LANGUAGE_NONE ? '' : ' ' . t($all[$lang]->name)),
      'index callback' => '_agileissues_get_field_values',
      'index arguments' => array($field_property, $name, $lang),
      'display callback' => '_agileissues_display_field_values',
      'display arguments' => array($display_callback, $name),
      'alter callback' => '_agileissues_alter_field_values',
      'include' => $include,
    );
  }
  return $processors;
}

function agileissues_construct_changelog_processor($display, $index, $index_display_callback = NULL, $include = 'internal.inc', $dynamic = FALSE) {
  $index_key = 'index';
  if ($dynamic) {
    $index_key = 'index callback';
  }
  return array(
    'display' => $display,
    $index_key => $index,
    'display callback' => $index_display_callback,
    'include' => $include,
  );
}

function agileissues_construct_relationship_definition($left, $right, $leftTypes = NULL, $rightTypes = NULL) {
  if (empty($leftTypes)) {
    $leftTypes = array('story', 'task');
  }
  elseif (!is_array($leftTypes)) {
    $leftTypes = array($leftTypes);
  }
  if (empty($rightTypes)) {
    $rightTypes = $leftTypes;
  }
  elseif (!is_array($rightTypes)) {
    $rightTypes = array($rightTypes);
  }
  return array(
    'left_display' => $left,
    'right_display' => $right,
    'left_types' => $leftTypes,
    'right_types' => $rightTypes,
  );
}

function agileissues_construct_search_option($description, $table, $index, $value, $search_type = NULL, $data_type = NULL, $index_type = 'property', $strict = TRUE) {
  if (empty($search_type)) {
    if ($table === 'task' || $table === 'sprint') {
      $search_type = array('task');
    }
    else {
      $search_type = array('task', 'story');
    }
  }
  elseif (!is_array($search_type)) {
    $search_type = array($search_type);
  }
  return array(
    'object' => $table,
    'applies_to' => $search_type,
    'property' => $index,
    'value' => $value,
    'type' => $index_type,
    'strict' => $strict,
    'data_type' => $data_type,
    'description' => $description,
  );
}

function agileissues_construct_search_parameter($description, $table, $index, $filter_function, $search_type = NULL, $data_type = NULL, $index_type = 'property', $strict = TRUE) {
  if (empty($search_type)) {
    if ($table === 'task') {
      $search_type = array('task');
    }
    else {
      $search_type = array('task', 'story');
    }
  }
  elseif (!is_array($search_type)) {
    $search_type = array($search_type);
  }
  return array(
    'object' => $table,
    'applies_to' => $search_type,
    'property' => $index,
    'sanitize' => $filter_function,
    'type' => $index_type,
    'strict' => $strict,
    'data_type' => $data_type,
    'description' => $description,
  );
}

function agileissues_get_story_priority_score($story) {
  $scores = module_invoke_all('agile_story_priority', $story);
  return agileissues_calculate_geometric_mean($scores);
}

function agileissues_calculate_geometric_mean($dataset) {
  $product = 0;
  $count = 0;
  foreach ($dataset as $score) {
    if (!empty($score)) {
      $product *= $score;
      $count++;
    }
  }
  if (empty($count)) { return 0; }
  return ceil(pow($product, 1 / $count));
}

function agileissues_get_task_actual_effort($taskID) {
  return array_sum(
    db_select('agileissues_notes', 'ain')
      ->fields('ain', array('id', 'time_spent'))
      ->condition('entity_type', 'task')
      ->condition('time_spent', 0, '>')
      ->condition('entity_id', $taskID)
      ->execute()->fetchAllKeyed(0, 1)
  );
}

function agileissues_get_story_actual_effort($storyID) {
  $taskIDs = db_select('agileissues_tasks', 'ait')
    ->fields('ait', array('id'))
    ->condition('story_id', $storyID)
    ->execute()->fetchAllKeyed(0, 0);
  return agileissues_get_task_actual_effort($taskIDs);
}

function agileissues_get_story_estimated_effort($storyID) {
  return array_sum(db_select('agileissues_tasks', 'ait')
    ->fields('ait', array('id', 'estimated_time'))
    ->condition('story_id', $storyID)
    ->execute()->fetchAllKeyed(0, 1));
}

function agileissues_get_story_usp_per_hour($story) {
  $usp = $story->story_points;
  $actualEffort = agileissues_get_story_actual_effort($story->id);
  if ($actualEffort > 0 && $usp > 0) {
    return $usp / ($actualEffort / 60);
  }
  return NULL;
}

function agileissues_get_usp_data($storyIDs) {
  $uphs = array();
  foreach ($storyIDs as $storyID) {
    $story = agileissues_story_load($storyID);
    $uph = agileissues_get_story_usp_per_hour($story);
    if ($uph !== NULL) {
      $uphs[$storyID] = $uph;
    }
  }
  return $uphs;
}

function agileissues_calculate_standard_metrics($dataset) {
  $metrics = array(
    'set' => $dataset,
    'mean' => agileissues_calculate_mean($dataset),
    'max' => max($dataset),
    'min' => min($dataset),
    'mode' => agileissues_calculate_mode($dataset),
    'median' => agileissues_calculate_median($dataset),
    'q1' => agileissues_calculate_q1($dataset),
    'q3' => agileissues_calculate_q3($dataset),
    'stdev' => agileissues_calculate_stddev($dataset),
    'z-score' => array(),
  );
  return $metrics;
}

function agileissues_calculate_stddev($dataset) {
  $x = agileissues_calculate_mean($dataset);
  $diffset = array();
  foreach ($dataset as $value) {
    $diffset[] = pow($value - $x, 2);
  }
  return pow(agileissues_calculate_mean($diffset), 0.5);
}

function agileissues_calculate_mode($dataset) {
  $count = array();
  foreach ($dataset as $value) {
    if (!isset($count[$value])) {
      $count[$value] = 0;
    }
    $count[$value]++;
  }
  $modes = array();
  $max = max($count);
  foreach ($count as $key => $value) {
    if ($value === $max) {
      $modes[] = $key;
    }
  }
  return $modes;
}

function agileissues_calculate_q1($dataset) {
  asort($dataset);
  $l = count($dataset);
  if (empty($l)) { return 0; }
  $lower = 0;
  if ($l % 2) {
    $lower = ($l - 3) / 2;
  }
  else {
    $lower = ($l - 2) / 2;
  }
  return agileissues_calculate_median(array_slice($dataset, 0, $lower));
}

function agileissues_calculate_q3($dataset) {
  asort($dataset);
  $l = count($dataset);
  if (empty($l)) { return 0; }
  $upper = 0;
  if ($l % 2) {
    $upper = ($l + 1) / 2;
  }
  else {
    $upper = ($l) / 2;
  }
  return agileissues_calculate_median(array_slice($dataset, $upper));
}

function agileissues_calculate_median($dataset) {
  asort($dataset);
  $l = count($dataset);
  if (empty($l)) { return 0; }
  $sdex = 0;
  $length = 1;
  if ($l % 2) {
    $length = 1;
    $sdex = ($l - 1) / 2;
  }
  else {
    $length = 2;
    $sdex = ($l - 2) / 2;
  }
  return agileissues_calculate_mean(array_slice($dataset, $sdex, $length));
}

function agileissues_calculate_mean($set) {
  $l = count($set);
  if (empty($l)) {
    return 0;
  }
  return array_sum($set) / $l;
}

function agileissues_load_relationships($object, $type) {
  $rels = array();
  $left_rels = db_select('agileissues_relationships', 'air')
    ->fields('air')
    ->condition('left_entity_id', $object->id)
    ->condition('left_entity_type', $type)
    ->execute()->fetchAll();
  foreach ($left_rels as $rel) {
    $typeKey = 'l|' . $rel->relationship_type;
    if (!isset($rels[$typeKey])) {
      $rels[$typeKey] = array(
        'type' => $typeKey,
        'objects' => array(),
      );
    }
    $obj_id = $rel->right_entity_type . '|' . $rel->right_entity_id;
    $rels[$typeKey]['objects'][$obj_id] = $obj_id;
  }
  $right_rels = db_select('agileissues_relationships', 'air')
    ->fields('air')
    ->condition('right_entity_id', $object->id)
    ->condition('right_entity_type', $type)
    ->execute()->fetchAll();
  foreach ($right_rels as $rel) {
    $typeKey = 'r|' . $rel->relationship_type;
    if (!isset($rels[$typeKey])) {
      $rels[$typeKey] = array(
        'type' => $typeKey,
        'objects' => array(),
      );
    }
    $obj_id = $rel->left_entity_type . '|' . $rel->left_entity_id;
    $rels[$typeKey]['objects'][$obj_id] = $obj_id;
  }
  $actual = array();
  foreach ($rels as $set) {
    $actual[] = array(
      'type' => $set['type'],
      'object' => implode(',', $set['objects']),
    );
  }
  return $actual;
}
