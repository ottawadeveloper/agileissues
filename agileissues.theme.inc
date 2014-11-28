<?php

function theme_agileissues_bar($vars) {
  $object = NULL;
  switch ($vars['type']) {
    case 'project':
      $object = $vars['project'];
      break;
    case 'story':
      $object = $vars['story'];
      break;
    case 'task':
      $object = $vars['task'];
      break;
  }
  $ops = agileissues_operations($vars['type'], $object, $vars['mode']);
  $output = '';
  $output .= '<ul class="agile-control-bar">';
  foreach ($ops as $op) {
    $output .= '<li>' . $op . '</li>';
  }
  $output .= '</ul>';
  drupal_add_css(drupal_get_path('module', 'agileissues') . '/misc/agileissues.css');
  return $output;
}

function theme_agileissues_backlog_name($vars) {
  if (empty($vars['backlog'])) { return ''; }
  return t($vars['backlog']->name);
}

function theme_agileissues_user_name($vars) {
  if (empty($vars['user'])) {
    return '';
  }
  return $vars['user']->name;
}

function theme_agileissues_sprint_name($vars) {
  if (empty($vars['sprint'])) { return ''; }
  return t('@sprintname (@sprintstart - @sprintend; @sprintstatus)', array(
    '@sprintname' => t($vars['sprint']->name),
    '@sprintstart' => format_date($vars['sprint']->start_date, 'short'),
    '@sprintend' => format_date($vars['sprint']->end_date, 'short'),
    '@sprintstatus' => $names[$vars['sprint']->status],
  ));
}
