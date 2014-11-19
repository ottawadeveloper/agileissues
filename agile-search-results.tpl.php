<?php

  $result_objects = entity_load($results['#result']['entity_type'], array_keys($results['#result']['ids']));
  
  echo '<div class="agile-search-results">';
  $view = entity_view($results['#result']['entity_type'], $result_objects, 'teaser');
  foreach ($view as $thing) {
    echo render($thing);
  }
  echo '</div>';