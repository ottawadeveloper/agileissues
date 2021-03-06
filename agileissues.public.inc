<?php

function agileissues_release_notes($project, $sprint = NULL) {
  $q = db_select('agileissues_sprints', 'ais')
    ->fields('ais')
    ->condition('project_id', $project->id);
  if (!empty($sprint)) {
    $q->condition('id', $sprint->id);
  }
  $q->orderBy('end_date', 'DESC');
  $sprints = $q->execute();
  $render = array(
    '#theme' => 'agileissues_release_notes',
    '#sprints' => array(),
    '#project' => $project,
  );
  foreach ($sprints as $sprint) {
    $render['#sprints'][$sprint->id] = $sprint;
  }
  return render($render);
}

function agileissues_vote_up($story, $type = 'ajax') {
  global $user;
  $vote = array(
    'story_id' => $story->id,
    'uid' => $user->uid,
  );
  db_merge('agileissues_story_user')
    ->key($vote)
    ->fields($vote)
    ->execute();
  drupal_set_message(t('Your vote has been recorded.'));
  if ($type !== 'ajax') {
    drupal_goto('agile-public');
  }
  else {
    ctools_include('ajax');
    module_load_include('internal.inc', 'agileissues');
    ajax_deliver(
      array(
        '#type' => 'ajax',
        '#commands' => array(
          ajax_command_replace('#story_vote_' . $story->id, _agileissues_generate_story_vote_link($story)),
        ),
      )
    );
    drupal_exit();
  }
}

function agileissues_vote_down($story, $type = 'ajax') {
  global $user;
  db_delete('agileissues_story_user')
    ->condition('uid', $user->uid)
    ->condition('story_id', $story->id)
    ->execute();
  drupal_set_message(t('Your vote has been removed.'));
  if ($type !== 'ajax') {
    drupal_goto('agile-public');
  }
  else {
    ctools_include('ajax');
    module_load_include('internal.inc', 'agileissues');
    ajax_deliver(
      array(
        '#type' => 'ajax',
        '#commands' => array(
          ajax_command_replace('#story_vote_' . $story->id, _agileissues_generate_story_vote_link($story)),
        ),
      )
    );
    drupal_exit();
  }
}

function agileissues_public_landing_page() {
  ctools_include('modal');
  ctools_modal_add_js();
  drupal_add_css(drupal_get_path('module', 'agileissues') . '/misc/agileissues.css');
  $projects = agileissues_project_load_all();
  $actualProjects = array();
  foreach ($projects as $k => $project) {
    if (agileissues_public_access('project', $project)) {
      $actualProjects[$k] = $project;
    }
  }
  return array(
    '#theme' => 'agileissues_public_frontend',
    '#projects' => entity_get_controller('agile_project')->view($actualProjects, 'teaser_public'),
  );
}

function agileissues_project_landing_page($project) {
  ctools_include('modal');
  ctools_modal_add_js();
  drupal_add_css(drupal_get_path('module', 'agileissues') . '/misc/agileissues.css');
  return entity_get_controller('agile_project')->view(array($project), 'public');
}

function agileissues_story_landing_page($story) {
  ctools_include('modal');
  ctools_modal_add_js();
  drupal_add_css(drupal_get_path('module', 'agileissues') . '/misc/agileissues.css');
  return entity_get_controller('agile_story')->view(array($story), 'public');
}

function agileissues_task_landing_page($task) {
  ctools_include('modal');
  ctools_modal_add_js();
  drupal_add_css(drupal_get_path('module', 'agileissues') . '/misc/agileissues.css');
  return entity_get_controller('agile_task')->view(array($task), 'public');
}

function agileissues_backlog_landing_page($project, $backlog = NULL) {
  ctools_include('modal');
  ctools_modal_add_js();
  drupal_add_css(drupal_get_path('module', 'agileissues') . '/misc/agileissues.css');
  $name = t('New');
  if (!empty($backlog)) {
    $name = t($backlog->name);
  }
  drupal_set_title(t($project->name) . ' - ' . $name);
  return array(
    '#theme' => 'agileissues_public_backlog_list',
    '#backlog' => $backlog,
    '#project' => $project,
  );
}

function agileissues_sprint_landing_page($project, $sprint = NULL) {
  ctools_include('modal');
  ctools_modal_add_js();
  drupal_add_css(drupal_get_path('module', 'agileissues') . '/misc/agileissues.css');
  $name = t('Unscheduled');
  if (!empty($sprint)) {
    $name = t($sprint->name);
  }
  drupal_set_title(t($project->name) . ' - ' . $name);
  return array(
    '#theme' => 'agileissues_public_sprint_list',
    '#sprint' => $sprint,
    '#project' => $project,
  );
}
