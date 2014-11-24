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