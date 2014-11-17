<?php


function agileissues_search_form($form, &$form_state, $searchFor = NULL, $searchIn = 'story') {
  $for = isset($form_state['values']['search']) ? $form_state['values']['search'] : $searchFor;
  $in = isset($form_state['values']['search-for']) ? $form_state['values']['search-for'] : $searchIn;
  $form['search'] = array(
    '#default_value' => $searchFor,
    '#title' => t('Query'),
    '#type' => 'textfield',
  );
  $form['search-for'] = array(
    '#type' => 'select',
    '#title' => t('Search in'),
    '#options' => array(
      'story' => t('Stories'),
      'task' => T('Tasks'),
    ),
    '#required' => TRUE,
    '#default_value' => $searchIn,
  );
  $form['go'] = array(
    '#type' => 'submit',
    '#value' => t('Search'),
  );
  if (!empty($for)) {
    module_load_include('search.inc', 'agileissues');
    $form['results'] = array(
      '#theme' => 'agileissues_search_results',
      '#result' => agileissues_execute_search($for, array(), $in),
    );
  }
  return $form;
}

function agileissues_search_form_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

function agileissues_get_content($callback, $ajax) {
  $args = func_get_args();
  array_shift($args);
  array_shift($args);
  if ($ajax) {
    ctools_include('modal');
    $output = drupal_render(call_user_func_array($callback, $args));
    $commands = array();
    // If the form has not yet been rendered, render it.
    $commands[] = ctools_modal_command_display(drupal_get_title(), $output);
    print ajax_render($commands);
    drupal_exit();
  }
  else {
    return call_user_func_array($callback, $args);
  }
}

function agileissues_story_page($story) {
  return entity_get_controller('agile_story')->view(array($story));
}

function agileissues_get_modal_form($form_id, $ajax) {
  $args = func_get_args();
  array_shift($args);
  array_shift($args);
  $form_state = array(
    'build_info' => array(
      'args' => $args,
    ),
  );
  if ($ajax) {
    ctools_include('ajax');
    ctools_include('modal');
    $form_state += array(
      'ajax' => TRUE,
    );
    
    $output = ctools_modal_form_wrapper($form_id, $form_state);
    if (!empty($form_state['ajax_commands'])) {
      $output = $form_state['ajax_commands'];
    }
    print ajax_render($output);
    drupal_exit();
  }
  else {
    return drupal_build_form($form_id, $form_state);
  }
}

function agileissues_frontend_page() {
  ctools_include('modal');
  ctools_modal_add_js();
  drupal_add_css(drupal_get_path('module', 'agileissues') . '/misc/agileissues.css');
  $projects = agileissues_project_load_all();
  $render = array();
  return array(
    '#theme' => 'agileissues_frontend',
    '#projects' => entity_get_controller('agile_project')->view($projects, 'teaser'),
  );
}


function agileissues_story_form($form, &$form_state, $story = NULL, $project = NULL) {
  global $user;
  $form['#attached'] = array(
    'css' => array(
      drupal_get_path('module', 'agileissues') . '/misc/agileissues.form.css',
    ),
  );
  $form['#theme'] = 'agileissues_story_form';
  if (empty($story)) {
    $defaultProject = 0;
    if (!empty($project)) {
      $defaultProject = $project->id;
    }
    elseif (isset($form_state['values']['project_id'])) {
      $defaultProject = $form_state['values']['project_id'];
    }
    $defaultBacklog = 0;
    if (!empty($project)) {
      $defaultBacklog = $project->default_backlog;
    }
    elseif (!empty($form_state['values']['backlog_id'])) {
      $defaultBacklog = $form_state['values']['backlog_id'];
    }
    $story = (object) array(
      'id' => 0,
      'type' => isset($form_state['values']['type']) ? $form_state['values']['type'] : '',
      'created' => 0,
      'title' => '',
      'backlog_id' => $defaultBacklog,
      'project_id' => $defaultProject,
      'weight' => 99999999,
      'primary_uid' => 0,
      'story_points' => 0,
      'reporting_uid' => $user->uid,
    );
  }
  if (empty($story->type)) {
    $form['project_id'] = array('#type' => 'value', '#value' => $story->project_id);
    $form['backlog_id'] = array('#type' => 'value', '#value' => $story->backlog_id);
    $form['type'] = array(
      '#type' => 'radios',
      '#title' => t('Story type'),
      '#options' => agileissues_load_bundles_of_type('story'),
      '#required' => TRUE,
    );
    $form['next'] = array(
      '#type' => 'submit',
      '#submit' => array('agileissues_rebuild_form_submit'),
      '#value' => t('Next'),
      '#validate' => array(),
    );
    return $form;
  }
  $form['id'] = array('#type' => 'value', '#value' => $story->id);
  $form['type'] = array('#type' => 'value', '#value' => $story->type);
  $form['created'] = array('#type' => 'value', '#value' => $story->created);
  $form['weight'] = array('#type' => 'value', '#value' => $story->weight);
  $form['reporting_uid'] = array('#type' => 'value', '#value' => $story->reporting_uid);
  $backlogs = array();
  foreach (agileissues_backlog_load_all() as $backlog) {
    $backlogs[$backlog->id] = t($backlog->name);
  }
  $projects = array();
  foreach (agileissues_project_load_all() as $project) {
    $projects[$project->id] = t($project->name);
  }
  $users = array();
  foreach (user_load_multiple(FALSE) as $user) {
    if (user_access('own agile stories', $user)) {
      $users[$user->uid] = $user->name;
    }
  }
  $form += array(
    'backlog_id' => array(
      '#type' => 'select',
      '#title' => t('Backlog'),
      '#options' => $backlogs,
      '#default_value' => $story->backlog_id,
      '#empty_value' => 0,
      '#empty_option' => t('(unassigned)'),
      '#weight' => -75,
    ),
    'project_id' => array(
      '#type' => 'select',
      '#title' => t('Project'),
      '#options' => $projects,
      '#default_value' => $story->project_id,
      '#empty_value' => 0,
      '#empty_option' => t('(unassigned)'),
      '#weight' => -100,
    ),
    'primary_uid' => array(
      '#type' => 'select',
      '#title' => t('Internal Contact'),
      '#options' => $users,
      '#default_value' => $story->primary_uid,
      '#empty_value' => 0,
      '#empty_option' => t('(unassigned)'),
      '#weight' => -50,
    ),
    'story_points' => array(
      '#type' => 'textfield',
      '#title' => t('Story Points'),
      '#default_value' => $story->story_points,
      '#size' => 5,
      '#weight' => -25,
    ),
  );
  $form['title'] = array(
    '#title' => t('Title'),
    '#type' => 'textfield',
    '#default_value' => $story->title,
    '#required' => TRUE,
  );
  
  $form['relationships'] = array(
    '#type' => 'fieldset',
    '#title' => t('Relationships'),
    '#tree' => TRUE,
  );
  $existing = array();
  if (!empty($story->id)) {
    $existing = agileissues_load_relationships($story, 'story');
  }
  if (empty($form_state['num_rels'])) {
    $form_state['num_rels'] = count($existing) + 1;
  }
  $relationship_types = array();
  foreach (agileissues_get_relationship_types() as $key => $type) {
    $relationship_types['l|' . $key] = $type['left_display'];
    $relationship_types['r|' . $key] = $type['right_display'];
  }
  ctools_include('ajax');
  for ($k = 0; $k < $form_state['num_rels']; $k++) {
    $form['relationships'][$k] = array(
      '#type' => 'fieldset',
      '#title' => t('Relationship @index', array(
        '@index' => $k + 1,
      )),
      'type' => array(
        '#type' => 'select',
        '#options' => $relationship_types,
        '#title' => t('Relationship Type'),
        '#default_value' => isset($existing[$k]) ? $existing[$k]['type'] : '',
      ),
      'object' => array(
        '#type' => 'textfield',
        '#title' => t('Related To'),
        '#size' => 30,
        '#autocomplete_path' => 'system/ajax/autocomplete-relationship',
        '#multiple' => TRUE,
        '#default_value' => isset($existing[$k]) ? $existing[$k]['object'] : '',
      ),
    );
  }
  field_attach_form('agile_story', $story, $form, $form_state);
 
  if (!empty($story->id)) {
    $form['change_notes'] = array(
      '#type' => 'textarea',
      '#title' => t('Change note'),
      '#required' => TRUE,
      '#weight' => 9999,
    );
  }
  
  $form['submit'] = array(
    '#weight' => 10000,
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

function agileissues_story_form_validate($form, &$form_state) {
  $object = agileissues_story_build_from_form($form_state);
  field_attach_form_validate('agile_story', $object, $form, $form_state);
}

function agileissues_story_build_from_form($form_state) {
  $properties = array('id', 'created', 'title', 'backlog_id', 'project_id', 'type', 'weight', 'reporting_uid', 'primary_uid', 'story_points');
  $stub_values = array();
  foreach ($properties as $key) {
    $stub_values[$key] = $form_state['values'][$key];
  }
  return (object) $stub_values;
}

function agileissues_save_relationships($object, $obj_type, $relationship_type, $relationship_input) {
  $relationships = explode(",", $relationship_input);
  list($direction, $type) = explode('|', $relationship_type, 2);
  foreach ($relationships as $relationship) {
    list($other_type, $other_id) = explode('|', $relationship, 2);
    if ($direction === 'l') {
      agileissues_define_relationship($obj_type, $object->id, $other_type, $other_id, $type);
    }
    else {
      agileissues_define_relationship($other_type, $other_id, $obj_type, $object->id, $type);
    }
  }
}

function agileissues_clear_relationships($object, $type) {
  db_delete('agileissues_relationships')
    ->condition('left_entity_id', $object->id)
    ->condition('left_entity_type', $type)
    ->execute();
  db_delete('agileissues_relationships')
    ->condition('right_entity_id', $object->id)
    ->condition('right_entity_type', $type)
    ->execute();
}

function agileissues_save_all_relationships($object, $type, &$form_state) {
  agileissues_clear_relationships($object, $type);
  foreach ($form_state['values']['relationships'] as $relationship) {
    if (!empty($relationship['type'])) {
      agileissues_save_relationships($object, $type, $relationship['type'], $relationship['object']);
    }
  }
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
  foreach ($rel as $set) {
    $actual[] = array(
      'type' => $set['type'],
      'object' => implode(',', $set['objects']),
    );
  }
  return $actual;
}

function agileissues_story_form_submit($form, &$form_state) {
  global $user;
  $object = agileissues_story_build_from_form($form_state);
  field_attach_submit('agile_story', $object, $form, $form_state);
  if (agileissues_story_save($object)) {
    agileissues_save_all_relationships($object, 'story', $form_state);
    if (!empty($form_state['values']['change_notes'])) {
      $note = (object) array(
        'entity_type' => 'story',
        'entity_id' => $object->id,
        'note_text' => $form_state['values']['change_notes'],
        'title' => 'Change note',
        'created' => time(),
        'updated' => time(),
        'uid' => $user->uid,
        'time_spent' => 0,
      );
      agileissues_save_note($note);
    }
    drupal_set_message(t('The story has been saved successfully. !addanother', array(
      '!addanother' => l(t('Add another'), 'agile/project/' . $object->project_id . '/open-request/nojs'),
    )));
    if ($form_state['ajax']) {
      ctools_include('ajax');
      $form_state['ajax_commands'][] = ctools_ajax_command_reload();
    } else {
      $form_state['redirect'] = 'agile/project/' . $object->project_id . '/my-reported-stories';
    }
  }
  else {
    drupal_set_message(t('There was an error.'), 'error');
    $form_state['rebuild'] = TRUE;
  }
}

function agileissues_ajax_reassign_story_backlog($story, $backlog) {
  return db_update('agileissues_stories')
    ->condition('id', $story->id)
    ->fields(array('backlog_id' => $backlog->id))
    ->execute() !== FALSE;
}

function agileissues_ajax_reassign_task_sprint($task, $backlog) {
  return db_update('agileissues_tasks')
    ->condition('id', $task->id)
    ->fields(array('backlog_id' => $backlog->id))
    ->execute() !== FALSE;
}

function agileissues_ajax_complete_task($task) {
  return db_update('agileissues_tasks')
    ->condition('id', $task->id)
    ->fields(array('status' => 'C'))
    ->execute() !== FALSE;
}

function agileissues_ajax_modify_backlog_status($backlog, $new_status) {
  return db_update('agileissues_backlog')
    ->condition('id', $backlog->id)
    ->fields(array('status' => $new_status))
    ->execute() !== FALSE;
}

function agileissues_story_move_form($form, &$form_state, $story) {
  $backlogs = array();
  foreach (agileissues_backlog_load_all() as $backlog) {
    $backlogs[$backlog->id] = t($backlog->name);
  }
  $form['story'] = array(
    '#type' => 'value',
    '#value' => $story,
  );
  $form['backlog_id'] = array(
    '#type' => 'select',
    '#title' => t('Backlog'),
    '#options' => $backlogs,
    '#default_value' => $story->backlog_id,
    '#empty_value' => 0,
    '#empty_option' => t('(unassigned)'),
  );
  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Move Story'),
  );
  return $form;
}

function agileissues_story_move_form_submit($form, &$form_state) {
  $backlog = agileissues_backlog_load($form_state['values']['backlog_id']);
  if (agileissues_ajax_reassign_story_backlog($form_state['values']['story'], $backlog)) {
    ctools_include('ajax');
    drupal_set_message(t('The story has been reassigned.'));
    $form_state['ajax_commands'][] = ctools_ajax_command_reload();
  }
  else {
    drupal_set_message(t('There was an error while reassigning the story.'), 'error');
    $form_state['rebuild'] = TRUE;
  }
}

function agileissues_note_form($form, &$form_state, $type, $object) {
  $form['entity_type'] = array(
    '#type' => 'value',
    '#value' => $type,
  );
  $form['entity_id'] = array(
    '#type' => 'value',
    '#value' => $object->id
  );
  $form['title'] = array(
    '#title' => t('Title'),
    '#type' => 'textfield',
  );
  $form['note_text'] = array(
    '#title' => t('Notes'),
    '#type' => 'textarea',
  );
  $form['time_spent'] = array(
    '#title' => t('Time Spent (hours)'),
    '#type' => 'textfield',
    '#default_value' => 0,
    '#size' => 3,
    '#access' => $type === 'task' && user_access('provide agile hours'),
  );
  $form['complete_task'] = array(
    '#type' => 'checkbox',
    '#title' => t('Mark this task as completed.'),
    '#access' => $type === 'task',
  );
  $form['create'] = array(
    '#value' => t('Create Note'),
    '#type' => 'submit',
  );
  return $form;
}

function agileissues_note_form_validate($form, &$form_state) {
  if (!empty($form_state['values']['time_spent']) && !filter_var($form_state['values']['time_spent'], FILTER_VALIDATE_FLOAT)) {
    form_set_error('time_spent', t('Time spent must be a number.'));
  }
}

function agileissues_note_form_submit($form, &$form_state) {
  global $user;
  $fields = array('entity_type', 'entity_id', 'title', 'note_text');
  $stub = array(
    'created' => time(),
    'updated' => time(),
    'uid' => $user->uid,
  );
  foreach ($fields as $key) {
    $stub[$key] = $form_state['values'][$key];
  }
  $stub['time_spent'] = $stub['time_spent'] * 60;
  $note = (object) $stub;
  if (agileissues_save_note($note)) {
    drupal_set_message(t('Note has been saved.'));
    if (!empty($form_state['values']['complete_task'])) {
      db_update('agileissues_tasks')
        ->condition('id', $form_state['values']['entity_id'])
        ->fields(array('status' => 'C'))
        ->execute();
    }
    ctools_include('ajax');
    $form_state['ajax_commands'][] = ctools_ajax_command_reload();
  }
  else {
    drupal_set_message(t('There was an error saving the note.'), 'error');
    $form_state['rebuild'] = TRUE;
  }
}

function agileissues_project_page($project) {
  return entity_get_controller('agile_project')->view(array($project));
}

function agileissues_manage_stories_form($form, &$form_state, $project, $backlog = NULL) {
  $q = db_select('agileissues_stories', 'ais')
    ->fields('ais', array('id'))
    ->orderBy('weight')
    ->condition('project_id', $project->id);
  $form['#backlog_name'] = t('Unassigned');
  if (!empty($backlog)) {
    $form['#backlog_name'] = t($backlog->name);
    $q->condition('backlog_id', $backlog->id);
  }
  else {
    $q->condition('backlog_id', 0);
  }  
  $stories = $q->execute()->fetchAllKeyed(0, 0);
  $form['#tree'] = TRUE;
  $form['#project'] = $project;
  
  $form['#theme'] = 'agileissues_manage_stories_form';
  $form['stories'] = array();
  foreach ($stories as $id) {
    $story = agileissues_story_load($id);
    $story_render = entity_get_controller('agile_story')->view(array($id => $story), 'teaser');
    $form['stories'][$id] = array(
      'weight' => array(
        '#type' => 'weight',
        '#title' => t('Weight'),
        '#title_display' => 'invisible',
        '#default_value' => $story->weight,
        '#delta' => 10,
      ),
      'render' => array(
        '#type' => 'markup',
        '#markup' => render($story_render),
      ),
    );
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save changes'),
  );
  return $form;
}

function agileissues_manage_stories_form_submit($form, &$form_state) {
  foreach ($form_state['values']['stories'] as $id => $data) {
    if (is_array($data) && isset($data['weight'])) {
      db_update('agileissues_stories')
        ->condition('id', $id)
        ->fields(array('weight' => $data['weight']))
        ->execute();
    }
  }
  drupal_set_message(t('The backlog order has been updated'));
}

function agileissues_task_form($form, &$form_state, $task = NULL, $story = NULL) {
  if (empty($task)) {
    $story_id = 0;
    if (!empty($story)) {
      $story_id = $story->id;
    }
    if (!empty($form_state['values']['story_id'])) {
      $story_id = $form_state['values']['story_id'];
    }
    $task = (object) array(
      'type' => isset($form_state['values']['type']) ? $form_state['values']['type'] : '',
      'title' => '',
      'story_id' => $story_id,
      'sprint_id' => 0,
      'weight' => 99999,
      'uid' => 0,
      'estimated_time' => 0,
      'status' => 'O',
      'created' => 0,
    );
  }
  if (empty($task->type)) {
    $types = agileissues_load_bundles_of_type('task');
    $form['type'] = array(
      '#type' => 'radios',
      '#title' => t('Task Type'),
      '#options' => $types,
      '#required' => TRUE,
    );
    $form['story_id'] = array(
      '#type' => 'value',
      '#value' => $task->story_id,
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Next'),
      '#submit' => array('agileissues_rebuild_form_submit'),
    );
    return $form;
  }
  $form['id'] = array(
    '#type' => 'value',
    '#value' => $task->id,
  );
  $form['weight'] = array(
    '#type' => 'value',
    '#value' => $task->weight,
  );
  $form['title'] = array(
    '#title' => t('Title'),
    '#type' => 'textfield',
    '#default_value' => $task->title,
    '#required' => TRUE,
  );
  $story_options = array();
  foreach (agileissues_story_load_multiple() as $story) {
    $story_options[$story->id] = $story->title;
  }
  $form['type'] = array(
    '#type' => 'value',
    '#value' => $task->type,
  );
  $form['story_id'] = array(
    '#options' => $story_options,
    '#title' => t('Story'),
    '#type' => 'select',
    '#required' => TRUE,
    '#default_value' => $task->story_id,
    '#access' => user_access('manage internal task data'),
  );
  $story_id = $task->story_id;
  if (!empty($form_state['values']['story_id'])) {
    $story_id = $form_state['values']['story_id'];
  }
  $sprints = array();
  if (!empty($story_id)) {
    $story = agileissues_story_load($story_id);
    if (!empty($story)) {
      $sprints = db_select('agileissues_sprints', 'ais')
        ->fields('ais', array('id', 'name'))
        ->condition('project_id', $story->project_id)
        ->execute()->fetchAllKeyed(0, 1);
    }
  }
  $form['sprint_id'] = array(
    '#type' => 'select',
    '#title' => t('Sprint'),
    '#options' => $sprints,
    '#disabled' => empty($sprints),
    '#default_value' => $task->sprint_id,
    '#empty_option' => t('Unscheduled'),
    '#empty_value' => 0,
    '#access' => user_access('manage internal task data'),
  );
  $user_options = array();
  foreach (user_load_multiple(FALSE) as $user) {
    if (user_access('own agile tasks')) {
      $user_options[$user->uid] = $user->name;
    }
  }
  $form['uid'] = array(
    '#title' => t('Owner'),
    '#type' => 'select',
    '#options' => $user_options,
    '#default_value' => $task->uid,
    '#empty_option' => t('Unassigned'),
    '#empty_value' => 0,
    '#access' => user_access('manage internal task data'),
  );
  $form['estimated_time'] = array(
    '#title' => t('Time Estimate (hours)'),
    '#type' => 'textfield',
    '#default_value' => round($task->estimated_time / 60, 2),
    '#size' => 3,
  );
  $form['status'] = array(
    '#title' => t('Status'),
    '#type' => 'select',
    '#options' => array(
      'O' => t('Open'),
      'C' => t('Closed'),
    ),
    '#required' => TRUE,
    '#default_value' => $task->status,
  );
  field_attach_form('agile_task', $task, $form, $form_state);
  if (!empty($task->id)) {
    $form['change_note'] = array(
      '#type' => 'textarea',
      '#title' => t('Change Note'),
      '#required' => TRUE,
      '#weight' => 9999,
    );
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 100000,
  );
  return $form;
}

function agileissues_task_form_submit($form, &$form_state) {
  global $user;
  $keys = array('title', 'id', 'weight', 'uid', 'created', 'type', 'story_id', 'sprint_id', 'uid', 'estimated_time', 'status');
  $stub = array();
  foreach ($keys as $key) {
    $stub[$key] = $form_state['values'][$key];
  }
  $stub['estimated_time'] = ceil($stub['estimated_time'] * 60);
  $entity = (object) $stub;
  field_attach_submit('agile_task', $entity, $form, $form_state);
  $new = TRUE;
  $changes = array();
  if (!empty($entity->id)) {
    $new = FALSE;
    $old = agileissues_task_load($entity->id);
    $changes = agileissues_process_changelog($old, $entity, 'task');
  }
  ctools_include('ajax');
  if (agileissues_save_task($entity)) {
    if (!$new) {
      $note = (object) array(
        'entity_type' => 'task',
        'entity_id' => $entity->id,
        'note_text' => $form_state['values']['change_note'],
        'title' => 'Change note',
        'created' => time(),
        'updated' => time(),
        'uid' => $user->uid,
        'time_spent' => 0,
      );
      agileissues_save_note($note);
      agileissues_save_changelog($note, $changes);
    }
    drupal_set_message(t('Task saved successfully.'));
    $form_state['ajax_commands'][] = ctools_ajax_command_reload();
  }
  else {
    drupal_set_message(t('An error occured while saving the task.'), 'error');
    $form_state['rebuild'] = TRUE;
  }
}

function agileissues_my_tasks_page($project = NULL) {
  global $user;
  $q = db_select('agileissues_tasks', 'ait')
    ->condition('uid', $user->uid)
    ->fields('ait', array('id'));
  if (!empty($project)) {
    $storyIDs = db_select('agileissues_stories', 'ais')
      ->fields('ais', array('id'))
      ->condition('project_id', $project->id)
      ->execute()->fetchAllKeyed(0, 0);
    $q->condition('story_id', $storyIDs, 'IN');
  }
  $ids = $q->execute()->fetchAllKeyed(0, 0);
  return array(
    '#theme' => 'agileissues_task_list',
    '#ids' => $ids,
    '#project' => $project,
  );
}

function agileissues_my_stories_page($project = NULL) {
  global $user;
  $q = db_select('agileissues_stories', 'ais')
    ->condition('primary_uid', $user->uid)
    ->fields('ais', array('id'));
  if (!empty($project)) {
    $q->condition('project_id', $project->id);
  }
  $ids = $q->execute()->fetchAllKeyed(0, 0);
  return array(
    '#theme' => 'agileissues_story_list',
    '#ids' => $ids,
    '#project' => $project,
  );
}

function agileissues_my_reported_stories_page($project = NULL) {
  global $user;
  $q = db_select('agileissues_stories', 'ais')
    ->condition('reporting_uid', $user->uid)
    ->fields('ais', array('id'));
  if (!empty($project)) {
    $q->condition('project_id', $project->id);
  }
  $ids = $q->execute()->fetchAllKeyed(0, 0);
  return array(
    '#theme' => 'agileissues_story_list',
    '#ids' => $ids,
    '#project' => $project,
  );
}

function agileissues_metrics_form($form, &$form_state, $project = NULL) {
  $metrics = agileissues_build_metrics_list();
  $options = array();
  foreach ($metrics as $key => $metric) {
    $options[$key] = $metric['display'];
  }
  if (empty($project)) {
    $project = $form_state['values']['project'];
  }
  $form['#theme'] = 'agileissues_metrics_form';
  $form['#project'] = $project;
  $form['project'] = array('#type' => 'value', '#value' => $project);
  $form['metric'] = array(
    '#options' => $options,
    '#type' => 'select',
    '#title' => t('Metric'),
    '#empty_opion' => t('Select...'),
    '#empty_value' => '',
    '#ajax' => array(
      'wrapper' => 'agile-metric-results',
      'callback' => 'agileissues_metrics_form_callback',
    ),
  );
  $form['result'] = array(
    '#prefix' => '<div id="agile-metric-results">',
    '#suffix' => '</div>',
    '#markup' => t('Please select a metric.'),
  );
  if (!empty($form_state['values']['metric'])) {
    $info = $metrics[$form_state['values']['metric']];
    $info['include'] += array(
      'page' => NULL,
      'type' => 'stats.inc',
    );
    module_load_include($info['include']['type'], $info['include']['module'], $info['include']['page']);
    $cb = $info['callback'];
    $form['results']['#markup'] = $cb($project);
  }
  return $form;
}

function agileissues_manage_tasks_form($form, &$form_state, $project, $sprint = NULL) {
  $q = db_select('agileissues_tasks', 'ait')
    ->fields('ait', array('id'))
    ->orderBy('weight');
  $storyIDs = db_select('agileissues_stories', 'ais')
    ->fields('ais', array('id'))
    ->condition('project_id', $project->id)
    ->execute()->fetchAllKeyed(0, 0);
  $q->condition('story_id', $storyIDs, 'IN');
  if (!empty($sprint)) {
    $q->condition('sprint_id', $sprint->id);
  }
  else {
    $q->condition('sprint_id', 0);
  }
  $taskIDs = $q->execute()->fetchAllKeyed(0, 0);
  $form['#sprint_name'] = t('Unscheduled');
  if (!empty($sprint)) {
    $form['#sprint_name'] = t($sprint->name);
    $q->condition('sprint_id', $sprint->id);
  }
  else {
    $q->condition('sprint_id', 0);
  }  
  $stories = $q->execute()->fetchAllKeyed(0, 0);
  $form['#tree'] = TRUE;
  $form['#project'] = $project;
  
  $form['#theme'] = 'agileissues_manage_tasks_form';
  $form['tasks'] = array();
  foreach ($taskIDs as $id) {
    $task = agileissues_task_load($id);
    $task_render = entity_get_controller('agile_task')->view(array($id => $task), 'teaser');
    $form['tasks'][$id] = array(
      'weight' => array(
        '#type' => 'weight',
        '#title' => t('Weight'),
        '#title_display' => 'invisible',
        '#default_value' => $task->weight,
        '#delta' => 10,
      ),
      'render' => array(
        '#type' => 'markup',
        '#markup' => render($task_render),
      ),
    );
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save changes'),
  );
  return $form;
}

function agileissues_task_page($story) {
  return entity_get_controller('agile_task')->view(array($story->id => $story));
}

function agileissues_task_assign_form($form, &$form_state, $task) {
  $form['task'] = array(
    '#type' => 'value',
    '#value' => $task,
  );
  $sprints = array();
  $story = agileissues_story_load($task->story_id);
  if (!empty($story)) {
    $sprints = db_select('agileissues_sprints', 'ais')
      ->fields('ais', array('id', 'name'))
      ->condition('project_id', $story->project_id)
      ->execute()->fetchAllKeyed(0, 1);
  }
  $form['sprint_id'] = array(
    '#type' => 'select',
    '#title' => t('Sprint'),
    '#options' => $sprints,
    '#disabled' => empty($sprints),
    '#default_value' => $task->sprint_id,
    '#empty_option' => t('Unscheduled'),
    '#empty_value' => 0,
    '#access' => user_access('manage internal task data'),
  );
  $user_options = array();
  foreach (user_load_multiple(FALSE) as $user) {
    if (user_access('own agile tasks')) {
      $user_options[$user->uid] = $user->name;
    }
  }
  $form['uid'] = array(
    '#title' => t('Owner'),
    '#type' => 'select',
    '#options' => $user_options,
    '#default_value' => $task->uid,
    '#empty_option' => t('Unassigned'),
    '#empty_value' => 0,
    '#access' => user_access('manage internal task data'),
  );
  $form['estimated_time'] = array(
    '#title' => t('Time Estimate (hours)'),
    '#type' => 'textfield',
    '#default_value' => round($task->estimated_time / 60, 2),
    '#size' => 3,
  );
  $form['submit'] = array(
    '#value' => t('Save'),
    '#type' => 'submit',
  );
  return $form;
}

function agileissues_task_assign_form_submit($form, &$form_state) {
  $keys = array('estimated_time', 'uid', 'sprint_id');
  $task = $form_state['values']['task'];
  foreach ($keys as $key) {
    $task->{$key} = $form_state['values'][$key];
  }
  $task->estimated_time = ceil($task->estimated_time * 60);
  ctools_include('ajax');
  if (agileissues_save_task($task)) {
    drupal_set_message(t('Task saved successfully.'));
    $form_state['ajax_commands'][] = ctools_ajax_command_reload();
  }
  else {
    drupal_set_message(t('An error occured while saving the task.'), 'error');
    $form_state['rebuild'] = TRUE;
  }
}

function agileissues_story_delete_form($form, &$form_state, $story) {
  $form['entity'] = array(
    '#type' => 'value',
    '#value' => $story,
  );
  $form = confirm_form($form, t('Are you sure you want to delete this story?'));
  return $form;
}

function agileissues_story_delete_form_submit($form, &$form_state) {
  $controller = entity_get_controller('agile_story');
  if ($controller->delete($form_state['values']['entity']->id)) {
    ctools_include('ajax');
    drupal_set_message(t('Story deleted successfully.'));
    $form_state['ajax_commands'][] = ctools_ajax_command_reload();
  }
  else {
    drupal_set_message(t('An error occurred while deleting the story.'), 'error');
    $form_state['rebuild'] = TRUE;
  }
}

function agileissues_task_delete_form($form, &$form_state, $task) {
  $form['entity'] = array(
    '#type' => 'value',
    '#value' => $task,
  );
  $form = confirm_form($form, t('Are you sure you want to delete this task?'), 'error');
  return $form;
}

function agileissues_task_delete_form_submit($form, &$form_state) {
  $controller = entity_get_controller('agile_task');
  $controller->delete(array($form_state['values']['entity']->id));
  ctools_include('ajax');
  drupal_set_message(t('Task deleted successfully.'));
  $form_state['ajax_commands'][] = ctools_ajax_command_reload();
}
