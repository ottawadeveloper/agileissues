<?php

  $output = theme('agileissues_bar', array(
    'type' => 'project',
    'project' => $list['#project'],
    'mode' => 'public',
  ));
  
  $output .= '<ul class="agile-backlog-horizontal-list">';
  
  if (agileissues_public_access('backlog', $list['#project'])) {
    $unassigned = agileissues_unassigned_stories($list['#project']);
    if (count($unassigned) > 0) {
      $render = array(
        '#theme' => 'agileissues_project_backlog_short',
        '#project' => $list['#project'],
        '#backlog' => NULL,
        '#items' => count($unassigned),
        '#public' => TRUE,
      );
      $output .= '<li>' . render($render) . '</li>';
    }
  }
  $backlogs = agileissues_project_backlogs($list['#project']);
  foreach ($backlogs as $backlog) {
    if (agileissues_public_access('backlog', $list['#project'], $backlog)) {
      $count = agileissues_project_count_stories($list['#project'], $backlog);
      if ($count > 0) {
        $render = array(
          '#theme' => 'agileissues_project_backlog_short',
          '#project' => $list['#project'],
          '#backlog' => $backlog,
          '#items' => $count,
          '#public' => TRUE,
        );
        $output .= '<li class="backlog-target" id="backlog_'.$backlog->id.'">' . render($render) . '</li>';
      }
    }
  }
    
  $output .= '</ul>';
  
  echo $output;
  
  $q = db_select('agileissues_stories', 'ais')
    ->fields('ais', array('id'))
    ->orderBy('weight')
    ->condition('project_id', $list['#project']->id);
  if (empty($list['#backlog'])) {
    $q->condition('backlog_id', 0);
  }
  else {
    $q->condition('backlog_id', $list['#backlog']->id);
  }
  $story_ids = $q->execute()->fetchAllKeyed(0, 0);
  $stories = entity_load('agile_story', $story_ids);
  
  echo '<div class="agile-story-list">';
  $render = entity_view('agile_story', $stories, 'teaser_public');
  echo render($render);
  echo '</div>';
  