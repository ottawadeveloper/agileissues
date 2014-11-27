<?php

  $temp = entity_load($results['#result']['entity_type'], array_keys($results['#result']['ids']));
  
  $mode = agileissues_frontend_access() ? 'teaser' : 'teaser-public';
  
  $result_objects = array();
  foreach ($temp as $id => $object) {
    $type = $results['#result']['entity_type'] === 'agile_story' ? 'story' : 'task';
    $valid = FALSE;
    if ($mode === 'teaser' && $type === 'story') {
      $valid = agileissues_story_access('view', $object);
    }
    elseif ($mode === 'teaser' && $type === 'task') {
      $valid = agileissues_task_access('view', $object);
    }
    else {
      $valid = agileissues_public_access($type, $object);
    }
    if ($valid) {
      $result_objects[$id] = $object;
    }
  }
  
  
  echo '<div class="agile-search-results">';
  $view = entity_view($results['#result']['entity_type'], $result_objects, $mode);
  foreach ($view as $thing) {
    echo render($thing);
  }
  echo '</div>';