<?php
  $link = 'agile/project/' . $elements['#project']->id . '/story-management';
  $name = t('Unassigned');
  if (!empty($elements['#backlog'])) {
    $link .= '/' . $elements['#backlog']->id;
    $name = t($elements['#backlog']->name);
  }
?>

<a href="<?php echo url($link); ?>" class="agile-backlog-link">
  <div class="agile-backlog-short">
    <h3><?php echo $name; ?> <span class="agile-backlog-short-items">(<?php echo $elements['#items']; ?>)</span></h3>
  </div>
</a>