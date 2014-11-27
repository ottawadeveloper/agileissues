<?php

  $temp = entity_load($results['#result']['entity_type'], array_keys($results['#result']['ids']));
  
  $mode = agileissues_frontend_access() ? 'teaser' : 'teaser-public';
  
  $result_objects = agileissues_filter_entities($results['#result']['entity_type'], $temp, $mode);
  
  echo '<div class="agile-search-results">';
  $view = entity_view($results['#result']['entity_type'], $result_objects, $mode);
  foreach ($view as $thing) {
    echo render($thing);
  }
  echo '</div>';
  