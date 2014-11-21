<?php

function agileissues_usp_sprint_list($project) {
  return __FUNCTION__;
}

function agileissues_usp_user_list($project) {
  return __FUNCTION__;
}

function agileissues_estimations_sprint_list($project) {
  return __FUNCTION__;
}

function agileissues_estimations_user_list($project) {
  return __FUNCTION__;
}

function agileissues_backlog_list($project) {
  return __FUNCTION__;
}

function agileissues_sprint_list($project) {
  $headers = array(
    'name' => 'Sprint',
    'open' => 'Open',
    'closed' => 'Closed',
  );
  $rows = array();
  $storyIDs = db_select('agileissues_stories', 'ais')
    ->condition('project_id', $project->id)
    ->fields('ais', array('id'))
    ->execute()->fetchAllKeyed(0, 0);
  $taskIDs = db_select('agileissues_tasks', 'ait')
    ->condition('story_id', $storyIDs)
    ->fields('ait', array('id'))
    ->execute()->fetchAllKeyed(0, 0);
  foreach ($taskIDs as $taskID) {
    $task = agileissues_task_load($taskID);
    $rowID = 0;
    if (!empty($task->sprint_id)) {
      $sprint = agileissues_sprint_load($task->sprint_id);
      $rowID = $sprint->id;
      if (empty($rows[$rowID])) {
        $rows[$rowID] = array(
          'name' => t($sprint->name),
          'open' => 0,
          'closed' => 0,
        );
      }
    }
    elseif (empty($rows[$rowID])) {
      $rows[$rowID] = array(
        'name' => t('Unscheduled'),
        'open' => 0,
        'closed' => 0,
      );
    }
    if ($task->status == 'O') {
      $rows[$rowID]['open']++;
    }
    else {
      $rows[$rowID]['closed']++;
    }
  }
  return theme('table', array(
    'header' => $headers,
    'rows' => $rows,
    'empty' => t('No tasks have been created for this project yet'),
  ));
}