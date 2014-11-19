<?php 

$options = array(
  'project' => $list['#project'],
  'type' => empty($list['#project']) ? 'global' : 'project',
);
echo theme('agileissues_bar', $options);
echo '<div id="agile-task-list">';
if (empty($list['#ids'])) {
  echo '<p>There are no tasks to be shown.</p>';
}
foreach ($list['#ids'] as $id) {
  $entity = entity_load('agile_task', array($id));
  $view = entity_view('agile_task', $entity, 'teaser');
  echo render($view);
}
echo '</div>';