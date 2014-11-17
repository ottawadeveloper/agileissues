<?php
  $link = 'agile/project/' . $elements['#project']->id . '/task-management';
  $name = t('Unassigned');
  if (!empty($elements['#sprint'])) {
    $link .= '/' . $elements['#sprint']->id;
    $name = t($elements['#sprint']->name);
  }
?>

<a href="<?php echo url($link); ?>" class="agile-sprint-link">
  <div class="agile-sprint-short">
    <h3><?php echo $name; ?> <span class="agile-sprint-short-items">(<?php echo $elements['#items']; ?>)</span></h3>
  </div>
</a>