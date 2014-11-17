<?php echo theme('agileissues_bar'); ?>
<ul class="agile-frontend-project-list">
  <?php
  foreach ($frontend['#projects'] as $project) {
    echo '<li>';
    echo render($project);
    echo '</li>';
  }
  ?>
</ul>