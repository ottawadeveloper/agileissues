<?php

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
  return array(
    '#theme' => 'agileissues_public_sprint_list',
    '#sprint' => $sprint,
    '#project' => $project,
  );
}
