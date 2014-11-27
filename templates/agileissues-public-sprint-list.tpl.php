<?php

  $output = theme('agileissues_bar', array(
    'type' => 'project',
    'project' => $list['#project'],
    'mode' => 'public',
  ));
  
  $output .= '<ul class="agile-sprint-horizontal-list">';
  
  if (agileissues_public_access('sprint', $list['#project'])) {
    $unassigned = agileissues_unassigned_tasks($list['#project']);
    if (count($unassigned) > 0) {
      $render = array(
        '#theme' => 'agileissues_project_sprint_short',
        '#project' => $list['#project'],
        '#backlog' => NULL,
        '#items' => count($unassigned),
      );
      $output .= '<li>' . render($render) . '</li>';
    }
  }
  $sprints = agileissues_project_sprints($list['#project']);
  foreach ($sprints as $sprint) {
    if (agileissues_public_access('sprint', $list['#project'], $sprint)) {
      $count = agileissues_project_count_tasks($list['#project'], $sprint);
      if ($count > 0) {
        $render = array(
          '#theme' => 'agileissues_project_sprint_short',
          '#project' => $list['#project'],
          '#sprint' => $sprint,
          '#items' => $count,
        );
        $output .= '<li class="sprint-target" id="sprint_'.$sprint->id.'">' . render($render) . '</li>';
      }
    }
  }
    
  $output .= '</ul>';
   
  echo $output;
  
  $story_ids = $ids = db_select('agileissues_stories', 'ais')
    ->fields('ais', array('id'))
    ->condition('project_id', $list['#project']->id)
    ->execute()
    ->fetchAllKeyed(0, 0);
  
  $q = db_select('agileissues_tasks', 'ais')
    ->fields('ais', array('id'))
    ->condition('story_id', $story_ids);
  if (empty($list['#sprint'])) {
    $q->condition('sprint_id', 0);
  }
  else {
    $q->condition('sprint_id', $list['#sprint']->id);
  }
  $task_ids = $q->execute()->fetchAllKeyed(0, 0);
  $tasks = entity_load('agile_task', $task_ids);
  
  echo '<div class="agile-task-list">';
  $render = entity_view('agile_task', $tasks, 'teaser_public');
  echo render($render);
  echo '</div>';
  