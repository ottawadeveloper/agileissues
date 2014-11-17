<?php
  drupal_add_css(drupal_get_path('module', 'agileissues') . '/misc/agileissues.css');
  
  echo theme('agileissues_bar', array(
    'type' => 'project',
    'project' => $form['#project'],
  ));
  
  echo drupal_render_children($form);