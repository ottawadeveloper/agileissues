<div class="agile-story-form">
  <?php if (isset($form['submit'])): ?>
    <?php
      $insetKeys = array(
        'project_id',
        'backlog_id',
        'primary_uid',
        'story_points',
        'field_impacted_audience',
        'field_busines_impact',
        'field_key_concerns',
        'field_target_date',
        'field_impacted_projects',
        'field_browser',
        'field_operating_system',
        'submit',
      );
      $html = '<div class="agile-story-form-inset">';
      
      foreach ($insetKeys as $key) {
        if (isset($form[$key])) {
          $html .= render($form[$key]); 
        }
      }
      $html .= '</div>';
    ?>
  <div class="agile-story-form-main">
    <?php echo drupal_render_children($form); ?>
  </div>
  <?php 
      echo $html;
    else: 
      echo drupal_render_children($form); 
    endif; 
  ?>
  
</div>
