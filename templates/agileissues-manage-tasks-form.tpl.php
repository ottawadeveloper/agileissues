<?php
  drupal_add_css(drupal_get_path('module', 'agileissues') . '/misc/agileissues.css');
  
  $output = theme('agileissues_bar', array(
    'type' => 'project',
    'project' => $form['#project'],
  ));
  
  $output .= '<ul class="agile-sprint-horizontal-list">';
  
  $unassigned = agileissues_unassigned_tasks($form['#project']);
  if (count($unassigned) > 0) {
    $render = array(
      '#theme' => 'agileissues_project_sprint_short',
      '#project' => $form['#project'],
      '#backlog' => NULL,
      '#items' => count($unassigned),
    );
    $output .= '<li>' . render($render) . '</li>';
  }
  $sprints = agileissues_project_sprints($form['#project']);
  foreach ($sprints as $sprint) {
    $count = agileissues_project_count_tasks($form['#project'], $sprint);
    if ($count > 0) {
      $render = array(
        '#theme' => 'agileissues_project_sprint_short',
        '#project' => $form['#project'],
        '#sprint' => $sprint,
        '#items' => $count,
      );
      $output .= '<li class="sprint-target" id="sprint_'.$sprint->id.'">' . render($render) . '</li>';
    }
  }
    
  $output .= '</ul>';
  $output .= '<h2>' . t($form['#sprint_name']) . '</h2>';
  $rows = array();
  
  foreach (element_children($form['tasks']) as $id) {
    $form['tasks'][$id]['weight']['#attributes']['class'] = array('task-weight');
    $rows[$id] = array(
      'data' => array(
        '',
        drupal_render($form['tasks'][$id]['weight']),
        drupal_render($form['tasks'][$id]['render']),
      ),
      'class' => array('task-draggable', 'draggable'),
      'id' => 'task_' . $id,
    );
  }
  
  $output .= theme('table', array(
    'header' => array('', '', ''),
    'rows' => $rows,
    'attributes' => array('id' => 'tasks-table'),
    'empty' => t('There are no tasks available in this bin.'),
  ));
  $output .= drupal_render_children($form);
  drupal_add_tabledrag('tasks-table', 'order', 'sibling', 'task-weight');
  
  echo $output;