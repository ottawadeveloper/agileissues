<?php

function theme_agileissues_bar($vars) {
  $ops = array();
  switch ($vars['type']) {
    case 'global':
      $ops = _agileissues_get_global_actions($vars['mode']);
      break;
    case 'project':
      $ops = _agileissues_project_operations($vars['project'], $vars['mode']);
      break;
    case 'story':
      $ops = _agileissues_story_operations($vars['story'], $vars['mode']);
      break;
    case 'task':
      $ops = _agileissues_task_operations($vars['task'], $vars['mode']);
      break;
  }
  $output = '';
  $output .= '<ul class="agile-control-bar">';
  foreach ($ops as $op) {
    $output .= '<li>' . $op . '</li>';
  }
  $output .= '</ul>';
  drupal_add_css(drupal_get_path('module', 'agileissues') . '/misc/agileissues.css');
  return $output;
}