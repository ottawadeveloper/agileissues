<?php

echo '<div class="agile-changenote">';
$user = user_load($change['#user']);
echo '<span class="agile-changenote-person">' . $user->name . '</span> changed ';
echo '<span class="agile-changenote-label">' . $change['#label'] . '</span> from ';
echo '<span class="agile-changenote-old">[' . $change['#old'] . ']</span> to';
echo '<span class="agile-changenote-new">[' . $change['#new'] . ']</span>';
echo '</div>';
