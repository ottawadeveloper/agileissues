<?php 

echo '<div class="agile-note-list">';
if (empty($list['#ids'])) {
  echo '<p>There are no notes to be shown.</p>';
}
else {
  echo '<ul>';
  foreach ($list['#children'] as $child) {
    echo render($child);
  }
  echo '</ul>';
}
echo '</div>';