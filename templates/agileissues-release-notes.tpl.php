<?php

global $language;

echo '<h1>' .t($notes['#project']->name) . ' - ' .  t('Release Notes') . '</h1>';

 echo theme('agileissues_bar', array(
    'type' => 'project',
    'project' => $notes['#project'],
  ));


function agileissues_sort_sprints_date($a, $b) {
  return $b->start_date - $a->start_date;
}

function agileissues_find_stories($sprint) {
  $ids = db_select('agileissues_stories', 'ais')
    ->condition('sprint_id', $sprint->id)
    ->orderBy('weight', 'ASC')
    ->fields('ais', array('id'))
    ->execute()->fetchAllKeyed(0, 0);
  return $ids;
}

function agileissues_sanitize($str) {
  $str = check_plain($str);
  $str = str_replace("\r", "\n", $str);
  $str = str_replace("\n\n", "\n", $str);
  $str = str_replace("\n", '<br />', $str);
  return $str;
}

uasort($notes['#sprints'], 'agileissues_sort_sprints_date');

foreach ($notes['#sprints'] as $id => $sprint) {
  if ($sprint->status !== 'P') {
    echo '<div class="agileissues-release-notes">';
    $full = agileissues_sprint_load($sprint->id);
    $date = '';
    if (!empty($full->field_release_date)) {
      $date = date(' - F jS Y', strtotime($full->field_release_date[LANGUAGE_NONE][0]['value']));
    }
    echo '<h2>' . t($sprint->name) . $date . '</h2>';

   $stories = agileissues_find_stories($sprint);
   if (empty($stories)) {
     echo '<p>' . t('There are no stories currently assigned to this release.') . '</p>';
   }
   else {
     echo '<ul>';
     foreach ($stories as $id) {
       echo '<li>';
       $story = agileissues_story_load($id);
       echo '<strong>#' . $story->id . ': '  . $story->title . '</strong>';
       if (!empty($story->field_release_notes[$language->language])) {
         echo '<br />' . agileissues_sanitize($story->field_release_notes[$language->language][0]['value']);
       }
       elseif (!empty($story->field_release_notes['en'])) {
         echo '<br /><span lang="en">' . agileissues_santize($story->field_release_notes['en'][0]['value']) . '</span>';
       }
       echo '</li>';
     }
     echo '</ul>';
   }
   echo '</div>';
  }
}
