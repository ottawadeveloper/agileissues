<?php

function agileissues_backend_page() {
  $html = array();
  $html[] = '<ul class="admin-list">';
  $html[] = '<li class="leaf">';
  $html[] = l(t('Backlogs'), 'admin/content/agile/backlog');
  $html[] = '<div class="description">';
  $html[] = 'Manage the backlogs that are available to the agile tracking system.';
  $html[] = '</div>';
  $html[] = '</li>';
  $html[] = '<li class="leaf">';
  $html[] = l(t('Projects'), 'admin/content/agile/project');
  $html[] = '<div class="description">';
  $html[] = 'Manage the projects that are available to the agile tracking system.';
  $html[] = '</div>';
  $html[] = '</li>';
  $html[] = '<li class="leaf">';
  $html[] = l(t('Sprints'), 'admin/content/agile/sprint');
  $html[] = '<div class="description">';
  $html[] = 'Manage the sprints that are available to the agile tracking system.';
  $html[] = '</div>';
  $html[] = '</li>';
  $html[] = '</ul>';
  return implode(PHP_EOL, $html);
}

function agileissues_projects_backend_page() {
  $rows = array();
  $projects = agileissues_project_load_all();
  foreach ($projects as $project) {
    $actions = array(
      l(t('edit'), 'admin/content/agile/project/' . $project->id),
      l(t('delete'), 'admin/content/agile/project/' . $project->id . '/delete'),
    );
    $rows[] = array(
      'name' => t($project->name),
      'actions' => implode(' | ', $actions),
    );
  }
  return theme('table', array(
    'rows' => $rows,
    'header' => array(
      'name' => t('Name'),
      'actions' => t('Actions'),
    ),
    'empty' => t('There are no projects defined yet.'),
  ));
}

function agileissues_project_form($form, &$form_state, $project = NULL) {
  if (empty($project)) {
    $project = (object) array(
      'id' => 0,
      'created' => 0,
      'name' => '',
      'default_backlog' => 0,
      'type' => isset($form_state['values']) && isset($form_state['values']['type']) ? $form_state['values']['type'] : NULL,
    );
  }
  if (empty($project->type)) {
    $form['type'] = array(
      '#type' => 'radios',
      '#options' => agileissues_load_bundles_of_type('project'),
      '#title' => t('Project Type'),
      '#empty_option' => t('Select...'),
      '#empty_value' => '',
      '#required' => TRUE,
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Next'),
      '#validate' => array(),
      '#submit' => array('agileissues_rebuild_form_submit'),
    );
    return $form;
  }
  $form['name'] = array(
    '#type' => 'textfield',
    '#maxlength' => 255,
    '#title' => t('Name'),
    '#default_value' => $project->name,
    '#required' => TRUE,
  );
  $form['id'] = array(
    '#type' => 'value',
    '#value' => $project->id,
  );
  $form['created'] = array(
    '#type' => 'value',
    '#value' => $project->created,
  );
  $form['type'] = array(
    '#type' => 'value',
    '#value' => $project->type,
  );
  $form['backlogs'] = array(
    '#type' => 'fieldset',
    '#title' => t('Backlog Settings'),
    '#weight' => 500,
    '#tree' => TRUE,
  );
  module_load_include('internal.inc', 'agileissues');
  $backlogs = agileissues_backlog_load_all();
  foreach ($backlogs as $backlog) {
    $options[$backlog->id] = t($backlog->name);
    $meta = (object) array(
      'used' => 'Y',
      'weight' => 0,
      'privacy' => 'P',
    );
    if (!empty($project->id)) {
      $meta = agileissues_load_backlog_project_metadata($project, $backlog);
    }
    $form['backlogs'][$backlog->id] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#title' => t($backlog->name),
      'used' => array(
        '#type' => 'checkbox',
        '#title' => t('Use this backlog as part of this project'),
        '#default_value' => $meta->used === 'Y' ? TRUE : FALSE,
      ),
      'weight' => array(
        '#type' => 'textfield',
        '#title' => t('Weight'),
        '#default_value' => $meta->weight,
      ),
      'privacy' => array(
        '#type' => 'options',
        '#title' => t('Privacy'),
        '#default_value' => $meta->privacy,
        '#required' => TRUE,
        '#options' => _agileissues_get_privacy_options(),
      ),
    );
  }
  $form['default_backlog'] = array(
      '#type' => 'select',
      '#default_value' => $project->default_backlog,
      '#empty_value' => 0,
      '#empty_option' => t('None'),
      '#options' => $options,
      '#title' => t('Default backlog'),
  );
  field_attach_form('agile_project', $project, $form, $form_state);
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 10000,
  );
  return $form;
}

function agileissues_project_form_validate($form, &$form_state) {
  $stub = agileissues_build_project_from_form($form_state);
  field_attach_form_validate('agile_project', $stub, $form, $form_state);
}

function agileissues_project_form_submit($form, &$form_state) {
  $stub = agileissues_build_project_from_form($form_state);
  field_attach_submit('agile_project', $stub, $form, $form_state);
  if (agileissues_project_save($stub)) {
    drupal_set_message(t('Project saved successfully!'));
    foreach ($form_state['values']['backlogs'] as $id => $backlog_metadata) {
      $meta = (object) $backlog_metadata;
      $meta->backlog_id = $id;
      $meta->project_id = $stub->project_id;
      db_merge('agileissues_project_backlog')
        ->key(array(
          'backlog_id' => $id,
          'project_id' => $stub->project_id
        ))
        ->fields((array) $meta)
        ->execute();
    }
    $form_state['redirect'] = 'admin/content/agile/project';
  }
  else {
    drupal_set_message(t('There was an error saving the project.'), 'error');
    $form_state['rebuild'] = TRUE;
  }
}

function agileissues_build_project_from_form(&$form_state) {
  $loadKeys = array('name', 'created', 'type', 'id', 'default_backlog');
  $values = array();
  foreach ($loadKeys as $key) {
    $values[$key] = $form_state['values'][$key];
  }
  return (object) $values;
}

function agileissues_project_delete_form($form, &$form_state, $project) {
  $form['project'] = array(
    '#type' => 'value',
    '#value' => $project,
  );
  $options = array();
  $projects = agileissues_project_load_all();
  foreach ($projects as $l) {
    if ($l->id !== $project->id) {
      $options[$l->id] = t($l->name);
    }
  }
  $form['reassign'] = array(
    '#type' => 'select',
    '#required' => FALSE,
    '#empty_option' => t('Delete all attached stories and tasks.'),
    '#title' => t('Reassign stories'),
    '#options' => $options,
    '#empty_value' => '',
    '#description' => t('You may reassign all the attached stories and tasks to a different project instead.'),
  );
  $form['reassign_sprint'] = array(
    '#type' => 'select',
    '#required' => FALSE,
    '#empty_option' => t('Delete all attached sprints.'),
    '#title' => t('Reassign sprints'),
    '#options' => $options,
    '#empty_value' => '',
    '#description' => t('You may reassign all the attached sprints to a different project instead.'),
  );
  return confirm_form($form, t('Are you sure you want to delete this project?'), 'admin/content/project/' . $project->id, t('You will not be able to undo this action!'));
}

function agileissues_project_delete_form_submit($form, &$form_state) {
  $tx = db_transaction();
  try {
    if (!empty($form_state['values']['reassign'])) {
      agileissues_reassign_story_project($form_state['values']['project']->id, $form_state['values']['reassign']);
    }
    if (!empty($form_state['values']['reassign_sprint'])) {
      agileissues_reassign_sprint_project($form_state['values']['project']->id, $form_state['values']['reasign_sprint']);
    }
    if (agileissues_project_delete($form_state['values']['project'], $tx)) {
      drupal_set_message(t('The project has been deleted successfully!'));
      $form_state['redirect'] = 'admin/content/agile/project';
    }
    else {
      drupal_set_message(t('There was an error'), 'error');
      $form_state['rebuild'] = TRUE;
    }
  }
  catch (Exception $e) {
    $tx->rollback();
    throw $e;
  }
  
}

function agileissues_backlogs_backend_page() {
  $rows = array();
  $backlogs = agileissues_backlog_load_all();
  foreach ($backlogs as $backlog) {
    $actions = array(
      l(t('edit'), 'admin/content/agile/backlog/' . $backlog->id),
      l(t('delete'), 'admin/content/agile/backlog/' . $backlog->id . '/delete'),
    );
    $rows[] = array(
      'name' => t($backlog->name),
      'actions' => implode(' | ', $actions),
    );
  }
  return theme('table', array(
    'rows' => $rows,
    'header' => array(
      'name' => t('Name'),
      'actions' => t('Actions'),
    ),
    'empty' => t('There are no backlogs defined yet.'),
  ));
}

function agileissues_backlog_form($form, &$form_state, $backlog = NULL) {
  if (empty($backlog)) {
    $backlog = (object) array(
      'id' => 0,
      'created' => 0,
      'name' => '',
      'type' => isset($form_state['values']) && isset($form_state['values']['type']) ? $form_state['values']['type'] : NULL,
    );
  }
  if (empty($backlog->type)) {
    $form['type'] = array(
      '#type' => 'radios',
      '#options' => agileissues_load_bundles_of_type('backlog'),
      '#title' => t('Backlog Type'),
      '#empty_option' => t('Select...'),
      '#empty_value' => '',
      '#required' => TRUE,
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Next'),
      '#validate' => array(),
      '#submit' => array('agileissues_rebuild_form_submit'),
    );
    return $form;
  }
  $form['name'] = array(
    '#type' => 'textfield',
    '#maxlength' => 255,
    '#title' => t('Name'),
    '#default_value' => $backlog->name,
    '#required' => TRUE,
  );
  $form['id'] = array(
    '#type' => 'value',
    '#value' => $backlog->id,
  );
  $form['created'] = array(
    '#type' => 'value',
    '#value' => $backlog->created,
  );
  $form['type'] = array(
    '#type' => 'value',
    '#value' => $backlog->type,
  );
  field_attach_form('agile_backlog', $backlog, $form, $form_state);
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 10000,
  );
  return $form;
}

function agileissues_backlog_form_validate($form, &$form_state) {
  $stub = agileissues_build_backlog_from_form($form_state);
  field_attach_form_validate('agile_backlog', $stub, $form, $form_state);
}

function agileissues_backlog_form_submit($form, &$form_state) {
  $stub = agileissues_build_backlog_from_form($form_state);
  field_attach_submit('agile_backlog', $stub, $form, $form_state);
  if (agileissues_backlog_save($stub)) {
    drupal_set_message(t('Backlog saved successfully!'));
    $form_state['redirect'] = 'admin/content/agile/backlog';
  }
  else {
    drupal_set_message(t('There was an error saving the backlog.'), 'error');
    $form_state['rebuild'] = TRUE;
  }
}

function agileissues_build_backlog_from_form(&$form_state) {
  $loadKeys = array('name', 'created', 'type', 'id');
  $values = array();
  foreach ($loadKeys as $key) {
    $values[$key] = $form_state['values'][$key];
  }
  return (object) $values;
}

function agileissues_backlog_delete_form($form, &$form_state, $backlog) {
  $form['backlog'] = array(
    '#type' => 'value',
    '#value' => $backlog,
  );
  $options = array();
  $backlogs = agileissues_backlog_load_all();
  foreach ($backlogs as $l) {
    if ($l->id !== $backlog->id) {
      $options[$l->id] = t($l->name);
    }
  }
  $form['reassign'] = array(
    '#type' => 'select',
    '#required' => FALSE,
    '#empty_option' => t('Delete all attached stories and tasks.'),
    '#title' => t('Reassign stories'),
    '#options' => $options,
    '#empty_value' => '',
    '#description' => t('You may reassign all the attached stories and tasks to a different backlog instead.'),
  );
  return confirm_form($form, t('Are you sure you want to delete this backlog?'), 'admin/content/backlog/' . $backlog->id, t('You will not be able to undo this action!'));
}

function agileissues_backlog_delete_form_submit($form, &$form_state) {
  $tx = db_transaction();
  try {
    if (!empty($form_state['values']['reassign'])) {
      agileissues_reassign_story_backlog($form_state['values']['backlog']->id, $form_state['values']['reassign'], $tx);
    }
    if (agileissues_backlog_delete($form_state['values']['backlog'], $tx)) {
      drupal_set_message(t('The backlog has been deleted successfully!'));
      $form_state['redirect'] = 'admin/content/agile/backlog';
    }
    else {
      drupal_set_message(t('There was an error'), 'error');
      $form_state['rebuild'] = TRUE;
    }
  }
  catch (Exception $e) {
    $tx->rollback();
    throw $e;
  }
  
}


function agileissues_sprints_backend_page() {
  $rows = array();
  $sprints = agileissues_sprint_load_all();
  uasort($sprints, '_agileissues_sort_sprints');
  $project_names = array();
  $projects = agileissues_project_load_all();
  foreach ($projects as $project) {
    $project_names[$project->id] = t($project->name);
  }
  $codes = _agileissues_get_sprint_status_codes();
  foreach ($sprints as $sprint) {
    $actions = array(
      l(t('edit'), 'admin/content/agile/sprint/' . $sprint->id),
      l(t('delete'), 'admin/content/agile/sprint/' . $sprint->id . '/delete'),
    );
    $rows[] = array(
      'name' => t($sprint->name),
      'project' => $project_names[$sprint->project_id],
      'status' => $codes[$sprint->status],
      'start' => format_date($sprint->start_date, 'custom', 'D M j, Y'),
      'end' => format_date($sprint->end_date, 'custom', 'D M j, Y'),
      'actions' => implode(' | ', $actions),
    );
  }
  return theme('table', array(
    'rows' => $rows,
    'header' => array(
      'name' => t('Name'),
      'project' => t('Project'),
      'status' => t('Status'),
      'start' => t('Start Date'),
      'end' => t('End Date'),
      'actions' => t('Actions'),
    ),
    'empty' => t('There are no sprints defined yet.'),
  ));
}

function _agileissues_sort_sprints($a, $b) {
  return $b->start_date - $a->start_date;
}

function _agileissues_get_sprint_status_codes() {
  return array(
    'P' => t('Planned'),
    'O' => t('Open'),
    'L' => t('Locked Down'),
    'C' => t('Complete'),
  );
}

function agileissues_sprint_form($form, &$form_state, $sprint = NULL) {
  if (empty($sprint)) {
    $sprint = (object) array(
      'id' => 0,
      'created' => 0,
      'project_id' => 0,
      'start_date' => time(),
      'end_date' => time() + (3600 * 24 * 14),
      'name' => '',
      'status' => 'P',
      'type' => isset($form_state['values']) && isset($form_state['values']['type']) ? $form_state['values']['type'] : NULL,
    );
  }
  if (empty($sprint->type)) {
    $form['type'] = array(
      '#type' => 'radios',
      '#options' => agileissues_load_bundles_of_type('sprint'),
      '#title' => t('Sprint Type'),
      '#empty_option' => t('Select...'),
      '#empty_value' => '',
      '#required' => TRUE,
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Next'),
      '#validate' => array(),
      '#submit' => array('agileissues_rebuild_form_submit'),
    );
    return $form;
  }
  $form['name'] = array(
    '#type' => 'textfield',
    '#maxlength' => 255,
    '#title' => t('Name'),
    '#default_value' => $sprint->name,
    '#required' => TRUE,
  );
  $form['id'] = array(
    '#type' => 'value',
    '#value' => $sprint->id,
  );
  $form['created'] = array(
    '#type' => 'value',
    '#value' => $sprint->created,
  );
  $form['type'] = array(
    '#type' => 'value',
    '#value' => $sprint->type,
  );
  $project_names = array();
  $projects = agileissues_project_load_all();
  foreach ($projects as $project) {
    $project_names[$project->id] = t($project->name);
  }
  $form['project_id'] = array(
      '#title' => t('Project'),
      '#type' => 'select',
      '#default_value' => $sprint->project_id,
      '#options' => $project_names,
      '#empty_value' => 0,
      '#empty_option' => t('Select'),
      '#required' => TRUE,
  );
  $form['start_date'] = array(
    '#title' => t('Start Date'),
    '#type' => 'date_popup',
    '#default_value' => date('Y-m-d H:i:s', $sprint->start_date),
    '#date_format' => 'Y-m-d',
  );
  $form['end_date'] = array(
    '#title' => t('End Date'),
    '#type' => 'date_popup',
    '#default_value' => date('Y-m-d H:i:s', $sprint->end_date),
    '#date_format' => 'Y-m-d',
  );
  $form['status'] = array(
    '#title' => t('Status'),
    '#type' => 'select',
    '#default_value' => $sprint->status,
    '#required' => TRUE,
    '#options' => _agileissues_get_sprint_status_codes(),
  );
  field_attach_form('agile_sprint', $sprint, $form, $form_state);
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 10000,
  );
  return $form;
}

function agileissues_sprint_form_validate($form, &$form_state) {
  $stub = agileissues_build_sprint_from_form($form_state);
  field_attach_form_validate('agile_sprint', $stub, $form, $form_state);
}

function agileissues_sprint_form_submit($form, &$form_state) {
  $stub = agileissues_build_sprint_from_form($form_state);
  field_attach_submit('agile_sprint', $stub, $form, $form_state);
  if (agileissues_sprint_save($stub)) {
    drupal_set_message(t('Backlog saved successfully!'));
    $form_state['redirect'] = 'admin/content/agile/sprint';
  }
  else {
    drupal_set_message(t('There was an error saving the sprint.'), 'error');
    $form_state['rebuild'] = TRUE;
  }
}

function agileissues_build_sprint_from_form(&$form_state) {
  $loadKeys = array('name', 'created', 'type', 'id', 'project_id', 'start_date', 'end_date', 'status');
  $values = array();
  foreach ($loadKeys as $key) {
    $values[$key] = $form_state['values'][$key];
  }
  return (object) $values;
}

function agileissues_sprint_delete_form($form, &$form_state, $sprint) {
  $form['sprint'] = array(
    '#type' => 'value',
    '#value' => $sprint,
  );
  $options = array();
  $sprints = agileissues_sprint_load_all();
  foreach ($sprints as $l) {
    if ($l->id !== $sprint->id) {
      $options[$l->id] = t($l->name);
    }
  }
  $form['reassign'] = array(
    '#type' => 'select',
    '#required' => FALSE,
    '#empty_option' => t('Delete all attached tasks.'),
    '#title' => t('Reassign tasks'),
    '#options' => $options,
    '#empty_value' => '',
    '#description' => t('You may reassign all the attached tasks to a different sprint instead.'),
  );
  return confirm_form($form, t('Are you sure you want to delete this sprint?'), 'admin/content/sprint/' . $sprint->id, t('You will not be able to undo this action!'));
}

function agileissues_sprint_delete_form_submit($form, &$form_state) {
  $tx = db_transaction();
  try {
    if (!empty($form_state['values']['reassign'])) {
      agileissues_reassign_task_sprint($form_state['values']['sprint']->id, $form_state['values']['reassign']);
    }
    if (agileissues_sprint_delete($form_state['values']['sprint'], $tx)) {
      drupal_set_message(t('The sprint has been deleted successfully!'));
      $form_state['redirect'] = 'admin/content/agile/sprint';
    }
    else {
      drupal_set_message(t('There was an error'), 'error');
      $form_state['rebuild'] = TRUE;
    }
  }
  catch (Exception $e) {
    $tx->rollback();
    throw $e;
  }
}
