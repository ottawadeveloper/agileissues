<?php 

$options = array(
  'project' => $list['#project'],
  'type' => empty($list['#project']) ? 'global' : 'project',
);
echo theme('agileissues_bar', $options);
echo '<div class="agile-story-list">';
if (empty($list['#ids'])) {
  echo '<p>There are no stories to be shown.</p>';
}
foreach ($list['#ids'] as $id) {
  $entity = entity_load('agile_story', array($id));
  $view = entity_view('agile_story', $entity, 'teaser');
  echo render($view);
}
echo '</div>';