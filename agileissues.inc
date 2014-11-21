<?php

function agileissues_get_story_priority_score($story) {
  $scores = module_invoke_all('agile_story_priority', $story);
  return agileissues_calculate_geometric_mean($scores);
}

function agileissues_calculate_geometric_mean($dataset) {
  $product = 0;
  $count = 0;
  foreach ($dataset as $score) {
    if (!empty($score)) {
      $product *= $score;
      $count++;
    }
  }
  if (empty($count)) { return 0; }
  return pow($product, 1 / $count);
}

function agileissues_get_task_actual_effort($taskID) {
  return array_sum(
    db_select('agileissues_notes', 'ain')
      ->fields('ain', array('id', 'time_spent'))
      ->condition('entity_type', 'task')
      ->condition('time_spent', 0, '>')
      ->condition('entity_id', $taskID)
      ->execute()->fetchAllKeyed(0, 1)
  );
}

function agileissues_get_story_actual_effort($storyID) {
  $taskIDs = db_select('agileissues_tasks', 'ait')
    ->fields('ait', array('id'))
    ->condition('story_id', $storyID)
    ->execute()->fetchAllKeyed(0, 0);
  return agileissues_get_task_actual_effort($taskIDs);
}

function agileissues_get_story_estimated_effort($storyID) {
  return array_sum(db_select('agileissues_tasks', 'ait')
    ->fields('ait', array('id', 'estimated_time'))
    ->condition('story_id', $storyID)
    ->execute()->fetchAllKeyed(0, 1));
}

function agileissues_get_story_usp_per_hour($story) {
  $usp = $story->story_points;
  $actualEffort = agileissues_get_story_actual_effort($story->id);
  if ($actualEffort > 0 && $usp > 0) {
    return $usp / ($actualEffort / 60);
  }
  return NULL;
}

function agileissues_get_usp_data($storyIDs) {
  $uphs = array();
  foreach ($storyIDs as $storyID) {
    $story = agileissues_story_load($storyID);
    $uph = agileissues_get_story_usp_per_hour($story);
    if ($uph !== NULL) {
      $uphs[$storyID] = $uph;
    }
  }
  return $uphs;
}

function agileissues_calculate_standard_metrics($dataset) {
  $metrics = array(
    'set' => $dataset,
    'mean' => agileissues_calculate_mean($dataset),
    'max' => max($dataset),
    'min' => min($dataset),
    'mode' => agileissues_calculate_mode($dataset),
    'median' => agileissues_calculate_median($dataset),
    'q1' => agileissues_calculate_q1($dataset),
    'q3' => agileissues_calculate_q3($dataset),
    'stdev' => agileissues_calculate_stddev($dataset),
    'z-score' => array(),
  );
  return $metrics;
}

function agileissues_calculate_stddev($dataset) {
  $x = agileissues_calculate_mean($dataset);
  $diffset = array();
  foreach ($dataset as $value) {
    $diffset[] = pow($value - $x, 2);
  }
  return pow(agileissues_calculate_mean($diffset), 0.5);
}

function agileissues_calculate_mode($dataset) {
  $count = array();
  foreach ($dataset as $value) {
    if (!isset($count[$value])) {
      $count[$value] = 0;
    }
    $count[$value]++;
  }
  $modes = array();
  $max = max($count);
  foreach ($count as $key => $value) {
    if ($value === $max) {
      $modes[] = $key;
    }
  }
  return $modes;
}

function agileissues_calculate_q1($dataset) {
  asort($dataset);
  $l = count($dataset);
  if (empty($l)) { return 0; }
  $lower = 0;
  if ($l % 2) {
    $lower = ($l - 3) / 2;
  }
  else {
    $lower = ($l - 2) / 2;
  }
  return agileissues_calculate_median(array_slice($dataset, 0, $lower));
}

function agileissues_calculate_q3($dataset) {
  asort($dataset);
  $l = count($dataset);
  if (empty($l)) { return 0; }
  $upper = 0;
  if ($l % 2) {
    $upper = ($l + 1) / 2;
  }
  else {
    $upper = ($l) / 2;
  }
  return agileissues_calculate_median(array_slice($dataset, $upper));
}

function agileissues_calculate_median($dataset) {
  asort($dataset);
  $l = count($dataset);
  if (empty($l)) { return 0; }
  $sdex = 0;
  $length = 1;
  if ($l % 2) {
    $length = 1;
    $sdex = ($l - 1) / 2;
  }
  else {
    $length = 2;
    $sdex = ($l - 2) / 2;
  }
  return agileissues_calculate_mean(array_slice($dataset, $sdex, $length));
}

function agileissues_calculate_mean($set) {
  $l = count($set);
  if (empty($l)) {
    return 0;
  }
  return array_sum($set) / $l;
}
