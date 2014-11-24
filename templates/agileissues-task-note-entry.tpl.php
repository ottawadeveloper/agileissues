<?php

echo '<div class="agile-change-task">';
$user = user_load($change['#task']->creator);
$username = t('Unknown');
if (!empty($user->uid)) {
  $username = $user->name;
}
echo '<span class="agile-change-task-person">' . $username . '</span> added task <span class="agile-change-task-link">';
echo l($change['#task']->title, 'agile/task/' . $change['#task']->id . '/nojs');
echo '</span> on <span class="agile-change-task-date">' . format_date($change['#agile_date'], 'medium') . '</span>';
echo '</div>';
