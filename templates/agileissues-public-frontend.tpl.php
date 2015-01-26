<?php echo theme('agileissues_bar', array('mode' => 'public')); ?>
<ul class="agile-frontend-project-list agile-frontend-project-list--public">
  <?php
  foreach ($frontend['#projects'] as $project) {
    echo '<li>';
    echo render($project);
    echo '</li>';
  }
  ?>
</ul>