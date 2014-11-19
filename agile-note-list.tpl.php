<?php 

echo '<div id="agile-note-list">';
if (empty($list['#ids'])) {
  echo '<p>There are no notes to be shown.</p>';
}
foreach ($list['#ids'] as $id) {
  $entity = entity_load('agile_note', array($id));
  $view = entity_view('agile_note', $entity, 'full');
  echo render($view);
}
echo '</div>';