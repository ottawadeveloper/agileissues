<h1>Search Tips</h1>
<h2>Stories</h2>
<dl>
  <?php 
    foreach ($tips['#stories'] as $tip => $def) {
      echo '<dt>' . $tip . '</dt><dd>' . $def . '</dd>';
    }
  ?>
</dl>
<h2>Tasks</h2>
<dl>
  <?php 
    foreach ($tips['#tasks'] as $tip => $def) {
      echo '<dt>' . $tip . '</dt><dd>' . $def . '</dd>';
    }
  ?>
</dl>