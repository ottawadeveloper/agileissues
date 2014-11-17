<?php
  drupal_add_css(drupal_get_path('module', 'agileissues') . '/misc/agileissues.css');
  
  $output = theme('agileissues_bar', array(
    'type' => 'project',
    'project' => $form['#project'],
  ));
  
  $output .= '<ul class="agile-backlog-horizontal-list">';
  
  $unassigned = agileissues_unassigned_stories($form['#project']);
  if (count($unassigned) > 0) {
    $render = array(
      '#theme' => 'agileissues_project_backlog_short',
      '#project' => $form['#project'],
      '#backlog' => NULL,
      '#items' => count($unassigned),
    );
    $output .= '<li>' . render($render) . '</li>';
  }
  $backlogs = agileissues_project_backlogs($form['#project']);
  foreach ($backlogs as $backlog) {
    $count = agileissues_project_count_stories($form['#project'], $backlog);
    if ($count > 0) {
      $render = array(
        '#theme' => 'agileissues_project_backlog_short',
        '#project' => $form['#project'],
        '#backlog' => $backlog,
        '#items' => $count,
      );
      $output .= '<li class="backlog-target" id="backlog_'.$backlog->id.'">' . render($render) . '</li>';
    }
  }
    
  $output .= '</ul>';
  $output .= '<h2>' . t($form['#backlog_name']) . '</h2>';
  $rows = array();
  
  foreach (element_children($form['stories']) as $id) {
    $form['stories'][$id]['weight']['#attributes']['class'] = array('story-weight');
    $rows[$id] = array(
      'data' => array(
        '',
        drupal_render($form['stories'][$id]['weight']),
        drupal_render($form['stories'][$id]['render']),
      ),
      'class' => array('story-draggable', 'draggable'),
      'id' => 'story_' . $id,
    );
  }
  
  $output .= theme('table', array(
    'header' => array('', '', ''),
    'rows' => $rows,
    'attributes' => array('id' => 'stories-table'),
    'empty' => t('There are no stories available in this bin.'),
  ));
  $output .= drupal_render_children($form);
  drupal_add_tabledrag('stories-table', 'order', 'sibling', 'story-weight');
  
  echo $output;