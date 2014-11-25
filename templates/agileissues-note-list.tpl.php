<?php 

echo '<div class="agile-note-list">';
if (empty($list['#children'])) {
  echo '<p>There are no notes to be shown.</p>';
}
else {
  echo '<ul>';
  foreach ($list['#children'] as $child) {
    echo '<li>' . render($child) . '</li>';
  }
  echo '</ul>';
}
echo '</div>';