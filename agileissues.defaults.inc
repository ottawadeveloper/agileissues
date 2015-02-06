<?php
/**
 * @file Contains default hook implementations to get them out of our business logic
 */

function _agileissues_default_relationships() {
  module_load_include('inc', 'agileissues');
  return array(
    'blocks' => agileissues_construct_relationship_definition(t('Blocks'), t('Blocked by')),
    'consult' => agileissues_construct_relationship_definition(t('See'), t('Is referenced by')),
    'duplicates' => agileissues_construct_relationship_definition(t('Duplicates'), t('Is duplicated by'), 'story'),
  );
}

function _agileissues_default_static_search_flags($object_type) {
  module_load_include('inc', 'agileissues');
  $flags = array();
  $flags['@noproject'] = agileissues_construct_search_option('Limit the results to stories that have no project.', 'story', 'project_id', 0);
  $flags['@nousp'] = agileissues_construct_search_option('Limit the results to stories with an empty story size.', 'story', 'story_points', 0);
  $flags['@unowned'] = agileissues_construct_search_option('Limit the results to stories without an internal contact.', 'story', 'primary_uid', 0, 'story');
  $flags['@nobacklog'] = agileissues_construct_search_option('Limit the results to stories which have not yet been assigned to a backlog.', 'story', 'backlog_id', 0);
  $flags['@storyid'] = agileissues_construct_search_parameter('Only show stories that match the given ID (eg @storyid=1).', 'story', 'id', FILTER_SANITIZE_NUMBER_INT);
  $flags['@backlogid'] = agileissues_construct_search_parameter('Only show stories that have been assigned to the given backlog (eg @backlogid=1).', 'backlog', 'id', FILTER_SANITIZE_NUMBER_INT);
  $flags['@backlog'] = agileissues_construct_search_parameter('Show stories that have been assigned to the given backlog (eg @backlog=Active).', 'backlog', 'name', 'check_plain', NULL, AGILE_SEARCH_DATA_TYPE_LIKE);
  $flags['@projectid'] = agileissues_construct_search_parameter('Only show stories that belong to the specified project (eg @projectid=1).', 'project', 'id', FILTER_SANITIZE_NUMBER_INT);
  $flags['@project'] = agileissues_construct_search_parameter('Show stories that have been assigned to the given project (eg @project="My Project").', 'backlog', 'name', 'check_plain', NULL, AGILE_SEARCH_DATA_TYPE_LIKE);
  $flags['@released'] = agileissues_construct_search_option('Limit the results to stories that have been released.', 'story', 'sprint_id', 0, NULL, AGILE_SEARCH_DATA_TYPE_NOT);
  $flags['@unreleased'] = agileissues_construct_search_option('Limit the results to stories that have not yet been released.', 'story', 'sprint_id', 0);
  if ($object_type === 'task') {
    $flags['@nostory'] = agileissues_construct_search_option('Limit the results to tasks that have no story.', 'task', 'story_id', 0);
    $flags['@unassigned'] = agileissues_construct_search_option('Limit the results to tasks which have not yet been assigned to a person.', 'task', 'uid', 0);
    $flags['@unscheduled'] = agileissues_construct_search_option('Limit the results to tasks which have not yet been assigned to a sprint.', 'task', 'sprint_id', 0);
    $flags['@taskid'] = agileissues_construct_search_parameter('Only show the specified task (eg @taskid=1).', 'task', 'id', FILTER_SANITIZE_NUMBER_INT);    
    $flags['@closed'] = agileissues_construct_search_option('Only show tasks that have been completed.', 'task', 'status', 'C');
    $flags['@completed'] = agileissues_construct_search_option('Alias of @closed.', 'task', 'status', 'C');
    $flags['@open'] = agileissues_construct_search_option('Only show tasks that are currently open.', 'task', 'status', 'O');
    $flags['@active'] = agileissues_construct_search_option('Alias of @open.', 'task', 'status', 'O');
    $flags['@planned'] = agileissues_construct_search_option('Only show tasks that are planned for future sprints.', 'sprint', 'status', 'P');
    $flags['@locked'] = agileissues_construct_search_option('Only show tasks that are about to be released.', 'sprint', 'status', 'L');
    $flags['@current'] = agileissues_construct_search_option('Only show tasks that are currently being worked on.', 'sprint', 'status', 'O');
    $flags['@releasedsprints'] = agileissues_construct_search_option('Only show tasks from sprints that have been released.', 'sprint', 'status', 'C');
    $flags['@oldsprints'] = agileissues_construct_search_option('Only show tasks from sprints that are about to or have been released.', 'sprint', 'status', array('C', 'L'));
    $flags['@sprintid'] = agileissues_construct_search_parameter('Only show tasks scheduled for the specified sprint ID (e.g. @sprintid=1).', 'sprint', 'id', FILTER_SANITIZE_NUMBER_INT);
    $flags['@sprint'] = agileissues_construct_search_parameter('Only show tasks scheduled for the specified sprint (e.g. @sprint="Sprint 7").', 'sprint', 'name', 'check_plain', NULL, AGILE_SEARCH_DATA_TYPE_LIKE);
    $flags['@story'] = agileissues_construct_search_parameter('Only show tasks scheduled for a story with a name similar to the one specified (e.g. @story="MySQL"). This', 'sprint', 'name', 'check_plain', NULL, AGILE_SEARCH_DATA_TYPE_LIKE);
  }
  else {
    $flags['@completed'] = agileissues_construct_search_option('Only show stories that have been completed.', 'story', 'status', 'C');
    $flags['@closed'] = agileissues_construct_search_option('Alias of @completed.', 'story', 'status', 'C');
    $flags['@active'] = agileissues_construct_search_option('Only show stories that are currently active.', 'story', 'status', 'A');
    $flags['@open'] = agileissues_construct_search_option('Alias of @active.', 'story', 'status', 'A');
    $flags['@sprintid'] = agileissues_construct_search_parameter('Only show stories that were released in the given sprint (eg @sprintid=1).', 'sprint', 'id', FILTER_SANITIZE_NUMBER_INT);
    $flags['@sprint'] = agileissues_construct_search_parameter('Only show stories that were released in the given sprint (e.g. @sprint="Sprint 7").', 'sprint', 'name', 'check_plain', NULL, AGILE_SEARCH_DATA_TYPE_LIKE);
  }
  return $flags;
}

function _agileissues_default_dynamic_search_flags($object_type) {
  global $user;
  module_load_include('inc', 'agileissues');
  $flags = array();
  $flags['@mystories'] = agileissues_construct_search_option('Limit results to stories that you opened.', 'story', 'reporting_uid', $user->uid);
  $flags['@myportfolio'] = agileissues_construct_search_option('Limit results to stories for which you are the internal contact.', 'story', 'primary_uid', $user->uid);
  if ($object_type === 'story') {
    $flags['@mine'] = agileissues_construct_search_option('Alias of @mystories', 'story', 'reporting_uid', $user->uid, 'story');
  }
  else {
    $flags['@mytasks'] = agileissues_construct_search_option('Limit results to tasks that are assigned to you.', 'task', 'uid', $user->uid);
    $flags['@mine'] = agileissues_construct_search_option('Alias of @mytasks', 'task', 'uid', $user->uid);
  }
  return $flags;
}

function _agileissues_default_changelog_processors($object_type) {
  module_load_include('inc', 'agileissues');
  switch ($object_type) {
    case 'story':
      return _agileissues_default_story_changelog_processors();
    case 'task':
      return _agileissues_default_task_changelog_processors();
    default:
      return array();
  }
}

function _agileissues_default_task_changelog_processors() {
  $processors = array();
  $processors['task-status'] = agileissues_construct_changelog_processor(t('Task Status'), 'status', '_agileissues_task_status_name');
  $processors['title'] = agileissues_construct_changelog_processor(t('Title'), 'title');
  $processors['story-id'] = agileissues_construct_changelog_processor(t('Story'), 'story_id', '_agileissues_task_story_name');
  $processors['sprint-id'] = agileissues_construct_changelog_processor(t('Sprint'), 'sprint_id', '_agileissues_sprint_name');
  $processors['uid'] = agileissues_construct_changelog_processor(t('Assignee'), 'uid', '_agileissues_user_name');
  $processors['estimated_time'] = agileissues_construct_changelog_processor(t('Estimated Time'), 'estimated_time', '_agileissues_format_time');
  $processors['privacy'] = agileissues_construct_changelog_processor(t('Privacy'), 'privacy', '_agileissues_get_privacy_name');
  $processors += _agileissues_default_field_processors('agile_task');
  return $processors;
}

function _agileissues_default_story_changelog_processors() {
  $processors = array();
  $processors['backlog'] = agileissues_construct_changelog_processor(t('Backlog'), 'backlog_id', '_agileissues_backlog_name');
  $processors['title'] = agileissues_construct_changelog_processor(t('Title'), 'title');
  $processors['project-id'] = agileissues_construct_changelog_processor(t('Project'), 'project_id', '_agileissues_project_name');
  $processors['primary-uid'] = agileissues_construct_changelog_processor(t('Internal Contact'), 'primary_uid', '_agileissues_user_name');
  $processors['story-points'] = agileissues_construct_changelog_processor(t('Story Size'), 'story_points', '_agileissues_story_size_name');
  $processors['status'] = agileissues_construct_changelog_processor(t('Status'), 'status', '_agileissues_get_story_status_name');
  $processors['sprint-id'] = agileissues_construct_changelog_processor(t('Completed'), 'sprint_id', '_agileissues_story_sprint_name');
  $processors['privacy'] = agileissues_construct_changelog_processor(t('Privacy'), 'privacy', '_agileissues_get_privacy_name');
  $processors += _agileissues_default_field_processors('agile_story');
  return $processors;
}

function _agileissues_default_field_processors($type) {
  $processors = array();
  $fields = field_info_instances($type);
  foreach ($fields as $type => $list) {
    foreach ($list as $name => $instance) {
      $field = field_info_field($name);
      switch ($field['type']) {
        case 'number_integer':
          $processors += agileissues_construct_changelog_field_processors($name, $field, $instance, 'value');
          break;
        case 'taxonomy_term_reference':
          $processors += agileissues_construct_changelog_field_processors($name, $field, $instance, 'tid', '_agileissues_taxonomy_term_name');
          break;
        case 'datetime':
        case 'date':
          $processors += agileissues_construct_changelog_field_processors($name, $field, $instance, 'value', '_agileissues_date_display');
          break;
        // other known fields that maybe we care about.
        case 'list_boolean':
        case 'link_field':
        case 'file':
        case 'text_long':
        case 'text_with_summary':
        case 'entityreference':
          break;
      }
    }
  }
  return $processors;
}

function _agileissues_default_operations($type, $object = NULL, $render = 'main') {
  switch ($type) {
    case 'project':
      return _agileissues_default_project_operations($object, $render);
    case 'story':
      return _agileissues_default_story_operations($object, $render);
    case 'task':
      return _agileissues_default_task_operations($object, $render);
    case 'global':
      return _agileissues_default_global_operations($render);
  }
  return array();
}

function _agileissues_default_project_operations($project, $render = 'main') {
  ctools_include('ajax');
  ctools_include('modal');
  ctools_modal_add_js();
  $ops = array();
  if (user_access('view release notes')) {
    $ops['release-notes'] = l(t('Release Notes'), 'agile-public/project/' . $project->id);
  }
  if (agileissues_public_access('project', $project) && $render === 'public') {
    $ops['view-public'] = l(t('Browse'), 'agile-public/project/' . $project->id);
  }
  if (agileissues_public_access('backlog', $project) && $render === 'public') {
    $ops['view-public-backlogs'] = l(t('Backlogs'), 'agile-public/project/' . $project->id . '/backlog');
  }
  if (agileissues_public_access('sprint', $project) && $render === 'public') {
    $ops['view-public-sprints'] = l(t('Sprints'), 'agile-public/project/' . $project->id . '/sprint');
  }
  if (agileissues_project_access($project)) {
    $ops['view'] = l($render === 'public' ? t('Manage') : t('View'), 'agile/project/' . $project->id);
  }
  if (agileissues_search_access()) {
    $ops['search'] = l(t('Search'), 'agile/search/@projectid=' . $project->id);
  }
  if (user_access('open new agile stories')) {
    $ops['open'] = l(t('New Story'), 'agile/project/' . $project->id . '/open-request/nojs', array('attributes' => array('class' => 'ctools-use-modal')));
    if ($render == 'main') {
      $ops['my-reported'] = l(t('My Stories'), 'agile/project/' . $project->id . '/my-reported-stories');
    }
  }
  if (user_access('own agile tasks') && $render == 'main') {
    $ops['my-tasks'] = l(t('My Tasks'), 'agile/project/' . $project->id . '/my-tasks');
  }
  if (user_access('own agile stories') && $render == 'main') {
    $ops['assigned-stories'] = l(t('My Portfolio'), 'agile/project/' . $project->id . '/my-stories');
  }
  if (user_access('manage agile stories')) {
    if (!empty($project->default_backlog)) {
      $ops['manage-stories'] = l(t('New Stories'), 'agile/project/' . $project->id . '/story-management/' . $project->default_backlog);
      $ops['orphan-stories'] = l(t('Orphan Stories'), 'agile/project/' . $project->id . '/story-management');
    }
    else {
      $ops['manage-stories'] = l(t('New Stories'), 'agile/project/' . $project->id . '/story-management');
    }
  }
  if (user_access('manage agile tasks')) {
    $ops['manage-tasks'] = l(t('Unscheduled Tasks'), 'agile/project/' . $project->id . '/task-management');
  }
  if (user_access('manage agile projects') && $render === 'main') {
    $ops['edit'] = l(t('Edit Project'), 'admin/content/agile/project/' . $project->id);
  }
  if (user_access('view agile statistics')) {
    $ops['stats'] = l(t('Metrics'), 'agile/project/' . $project->id . '/stats');
  }
  return $ops;
}

function _agileissues_default_story_operations($story, $render = 'main') {
  global $user;
  ctools_include('ajax');
  ctools_include('modal');
  ctools_modal_add_js();
  $ops = array();
  if ($render ==='public' && agileissues_public_access('story', $story)) {
    $ops[] = l(t('View'), 'agile-public/story/' . $story->id . '/nojs', array('attributes' => array('class' => 'ctools-use-modal')));
  }
  elseif (agileissues_story_access('view', $story)) {
    $ops[] = l(t('View'), 'agile/story/' . $story->id . '/nojs', array('attributes' => array('class' => 'ctools-use-modal')));
  }
  if (user_access('vote on agile stories')) {
    module_load_include('internal.inc', 'agileissues');
    $ops[] = _agileissues_generate_story_vote_link($story);
  }
  if (agileissues_story_access('edit', $story)) {
    $ops[] = l(t('Edit'), 'agile/story/' . $story->id . '/edit/nojs', array('attributes' => array('class' => 'ctools-use-modal')));
  }
  if (agileissues_story_access('move', $story)) {
    $ops[] = l(t('Move'), 'agile/story/' . $story->id . '/move/nojs', array('attributes' => array('class' => 'ctools-use-modal')));
  }
  if (user_access('add agile story notes')) {
    $ops[] = l(t('Add Note'), 'agile/story/' . $story->id . '/note/nojs', array('attributes' => array('class' => 'ctools-use-modal')));
  }
  if (user_access('open new agile tasks')) {
    $ops[] = l(t('Add Task'), 'agile/story/' . $story->id . '/task/nojs', array('attributes' => array('class' => 'ctools-use-modal')));
  }
  if (agileissues_story_access('delete', $story)) {
    $query = array('query' => drupal_get_destination());
    $ops[] = l(t('Delete'), 'agile/story/' . $story->id . '/delete/nojs', array('attributes' => array('class' => 'ctools-use-modal')) + $query);
  }
  return $ops;
}

function _agileissues_default_task_operations($task, $render = 'main') {
  ctools_include('ajax');
  ctools_include('modal');
  ctools_modal_add_js();
  $ops = array();
  if ($render === 'public' && agileissues_public_access('task', $task)) {
    $ops[] = l(t('View'), 'agile-public/task/' . $task->id . '/nojs', array('attributes' => array('class' => 'ctools-use-modal')));
  }
  elseif (agileissues_task_access('view', $task)) {
    $ops[] = l(t('View'), 'agile/task/' . $task->id . '/nojs', array('attributes' => array('class' => 'ctools-use-modal')));
  }
  if (agileissues_task_access('edit', $task)) {
    $ops[] = l(t('Edit'), 'agile/task/' . $task->id . '/edit/nojs', array('attributes' => array('class' => 'ctools-use-modal')));
  }
  if (agileissues_task_access('note', $task)) {
    $ops[] = l(t('Add Note'), 'agile/task/' . $task->id . '/note/nojs', array('attributes' => array('class' => 'ctools-use-modal')));
  }
  if (user_access('manage internal task data')) {
    $ops[] = l(t('Assign'), 'agile/task/' . $task->id . '/assign/nojs', array('attributes' => array('class' => 'ctools-use-modal')));
  }
  if (!empty($task->story_id)) {
    $story = agileissues_story_load($task->story_id);
    if (agileissues_story_access('view', $story)) {
      $ops[] = l(t('View Story'), 'agile/story/' . $task->story_id . '/nojs', array('attributes' => array('class' => 'ctools-use-modal')));
    }
  }
  if (agileissues_task_access('delete', $task)) {
    $query = array('query' => drupal_get_destination());
    $ops[] = l(t('Delete'), 'agile/task/' . $task->id . '/delete/nojs', array('attributes' => array('class' => 'ctools-use-modal')) + $query);
  }
  return $ops;
}

function _agileissues_default_global_operations($render = 'main') {
  ctools_include('ajax');
  ctools_include('modal');
  ctools_modal_add_js();
  $actions = array();
  if (agileissues_frontend_access()) {
    $actions[] = l(t('Projects'), 'agile');
  }
  if (agileissues_public_access('home')) {
    $actions[] = l(t('Browse'), 'agile-public');
  }
  if (agileissues_search_access()) {
    $actions[] = l(t('Search'), 'agile/search');
    $actions[] = l(t('Search Tips'), 'agile/search-tips/nojs', array('attributes' => array('class' => 'ctools-use-modal')));
  }
  if (user_access('open new agile stories')) {
    $actions[] = l(t('New Story'), 'agile/open-request/nojs', array('attributes' => array('class' => 'ctools-use-modal')));
    $actions[] = l(t('My Stories'), 'agile/my-reported-stories');
  }
  if (user_access('own agile stories')) {
    $actions[] = l(t('My Portfolio'), 'agile/my-stories');
  }
  if (user_access('own agile tasks')) {
    $actions[] = l(t('My Tasks'), 'agile/my-tasks');
  }
  return $actions;
}

function _agileissues_default_metrics() {
   $file = array(
    'include' => 'stats.inc',
  );
  return array(
    'agileissues_sprint_list' => array(
      'callback' => 'agileissues_sprint_list',
      'display' => t('Tasks by sprint'),
    ) + $file,
    'agileissues_backlog_list' => array(
      'callback' => 'agileissues_backlog_list',
      'display' => t('Stories by backlog'),
    ) + $file,
    'agileissues_usp_sprint' => array(
      'callback' => 'agileissues_usp_sprint_list',
      'display' => t('USP completed by sprint'),
    ) + $file,
    'agileissues_usp_user' => array(
      'callback' => 'agileissues_usp_user_list',
      'display' => t('USP completed by user'),
    ),
    'agileissues_estimations_sprint' => array(
      'callback' => 'agileissues_estimations_sprint_list',
      'display' => t('Time estimations by sprint'),
    ),
    'agileissues_estimations_user' => array(
      'callback' => 'agileissues_estimations_user_list',
      'display' => t('Time estimations by user'),
    ),
  );
}

function _agileissues_default_theme_hooks($path) {
  $path .= '/templates';
  return array(
    'agileissues_sprint_name' => array(
      'variables' => array(
        'sprint' => NULL,
      ),
      'file' => 'agileissues.theme.inc',
    ),
    'agileissues_backlog_name' => array(
      'variables' => array(
        'backlog' => NULL,
      ),
      'file' => 'agileissues.theme.inc',
    ),
    'agileissues_public_frontend' => array(
      'render element' => 'frontend',
      'template' => 'agileissues-public-frontend',
      'path' => $path,
    ),
    'agileissues_public_backlog_list' => array(
      'render element' => 'list',
      'template' => 'agileissues-public-backlog-list',
      'path' => $path,
    ),
    'agileissues_public_sprint_list' => array(
      'render element' => 'list',
      'template' => 'agileissues-public-sprint-list',
      'path' => $path,
    ),
    'agileissues_search_tips' => array(
      'render element' => 'tips',
      'template' => 'agileissues-search-tips',
      'path' => $path,
    ),
    'agileissues_task_note_entry' => array(
      'render element' => 'change',
      'template' => 'agileissues-task-note-entry',
      'path' => $path,
    ),
    'agileissues_changelog_entry' => array(
      'render element' => 'change',
      'template' => 'agileissues-changelog-entry',
      'path' => $path,
    ),
    'agileissues_task_form' => array(
      'render element' => 'form',
      'template' => 'agileissues-task-form',
      'path' => $path,
    ),
    'agileissues_metrics_form' => array(
      'render element' => 'form',
      'template' => 'agileissues-metrics-form',
      'path' => $path,
    ),
    'agileissues_note_list' => array(
      'render element' => 'list',
      'template' => 'agileissues-note-list',
      'path' => $path,
    ),
    'agileissues_bar' => array(
      'variables' => array(
        'project' => NULL,
        'story' => NULL,
        'task' => NULL,
        'mode' => 'main',
        'type' => 'global',
      ),
      'file' => 'agileissues.theme.inc',
    ),
    'agileissues_story_list' => array(
      'render element' => 'list',
      'template' => 'agileissues-story-list',
      'path' => $path,
    ),
    'agileissues_task_list' => array(
      'render element' => 'list',
      'template' => 'agileissues-task-list',
      'path' => $path,
    ),
    'agileissues_frontend' => array(
      'render element' => 'frontend',
      'template' => 'agileissues-frontend',
      'path' => $path,
    ),
    'agileissues_project_backlog_short' => array(
      'render element' => 'elements',
      'template' => 'agileissues-project-backlog-short',
      'path' => $path,
    ),
    'agileissues_project_sprint_short' => array(
      'render element' => 'elements',
      'template' => 'agileissues-project-sprint-short',
      'path' => $path,
    ),
    'agileissues_story_form' => array(
      'render element' => 'form',
      'template' => 'agileissues-story-form',
      'path' => $path,
    ),
    'agileissues_manage_stories_form' => array(
      'render element' => 'form',
      'template' => 'agileissues-manage-stories-form',
      'path' => $path,
    ),
    'agileissues_manage_tasks_form' => array(
      'render element' => 'form',
      'template' => 'agileissues-manage-tasks-form',
      'path' => $path,
    ),
    'agileissues_metrics_form' => array(
      'render element' => 'form',
      'template' => 'agileissues-metrics-form',
      'path' => $path,
    ),
    'agileissues_search_form' => array(
      'render element' => 'form',
      'template' => 'agileissues-search-form',
      'path' => $path,
    ),
    'agileissues_search_results' => array(
      'render element' => 'results',
      'template' => 'agileissues-search-results',
      'path' => $path,
    ),
    'agileissues_release_notes' => array(
      'render element' => 'notes',
      'template' => 'agileissues-release-notes',
      'path' => $path,
    ),
    'agileissues_release_sprint' => array(
      'render element' => 'sprint',
      'template' => 'agileissues-release-sprint',
      'path' => $path,
    ),
    'agileissues_release_story' => array(
      'render element' => 'story',
      'template' => 'agileissues-release-story',
      'path' => $path,
    ),
  );
}

function _agileissues_default_permissions() {
  return array(
    'access internal objects' => array(
      'title' => t('View all internal objects'),
    ),
    'view release notes' => array(
      'title' => t('View the release notes'),
    ),
    'access all hidden objects' => array(
      'title' => t('Access all hidden objects, regardless of owner.'),
    ),
    'browse public agile stories' => array(
      'title' => t('Browse the public list of stories'),
    ),
    'browse public agile tasks' => array(
      'title' => t('Browse the public list of tasks'),
    ),
    'manage agile entities' => array(
      'title' => t('Manage agile entities'),
    ),
    'manage agile projects' => array(
      'title' => t('Manage agile projects'),  
    ),
    'manage agile backlogs' => array(
      'title' => t('Manage agile backlogs'),
    ),
    'manage agile sprints' => array(
      'title' => t('Manage agile sprints'),
    ),
    'view own agile stories' => array(
      'title' => t('View own agile stories'),
    ),
    'view all agile stories' => array(
      'title' => t('View all agile stories'),
    ),
    'edit own agile stories' => array(
      'title' => t('Edit own agile stories'),
    ),
    'edit all agile stories' => array(
      'title' => t('Edit all agile stories'),
    ),
    'open new agile stories' => array(
      'title' => t('Open new agile stories'),
    ),
    'delete agile stories' => array(
      'title' => t('Delete agile stories'),
    ),
    'manage internal story data' => array(
      'title' => t('Manage internal story data'),
    ),
    'add agile task notes' => array(
      'title' => t('Add agile task notes'),
    ),
    'add agile story notes' => array(
      'title' => t('Add agile story notes'),
    ),
    'provide agile hours' => array(
      'title' => t('Provide agile hours'),
    ),
    'view own agile tasks' => array(
      'title' => t('View own agile tasks'),
    ),
    'view all agile tasks' => array(
      'title' => t('View all agile tasks'),
    ),
    'edit own agile tasks' => array(
      'title' => t('Edit own agile tasks'),
    ),
    'edit all agile tasks' => array(
      'title' => t('Edit all agile tasks'),
    ),
    'open new agile tasks' => array(
      'title' => t('Open new agile tasks'),
    ),
    'delete agile tasks' => array(
      'title' => t('Delete agile tasks'),
    ),
    'manage internal task data' => array(
      'title' => t('Manage internal task data'),
    ),
    'own agile tasks' => array(
      'title' => t('Become the owner of agile tasks'),
    ),
    'own agile stories' => array(
      'title' => t('Become the owner of agile stories'),
    ),
    'manage agile stories' => array(
      'title' => t('Manage agile stories'),
    ),
    'manage agile tasks' => array(
      'title' => t('Manage agile tasks'),
    ),
    'view agile statistics' => array(
      'title' => t('View agile statistics'),
    ),
    'change note privacy' => array(
      'title' => t('Change note privacy'),
    ),
    'vote on agile stories' => array(
      'title' => t('Vote on agile stories'),
    ),
  );
}

function _agileissues_default_menu() {
  $menu = array(
    'system/agile/vote-down/%agileissues_story' => array(
      'title' => t('Vote Down Agile'),
      'file' => 'agileissues.public.inc',
      'page callback' => 'agileissues_vote_down',
      'page arguments' => array(3),
      'access arguments' => array('vote on agile stories'),
    ),
    'system/agile/vote-up/%agileissues_story' => array(
      'title' => t('Vote Up Agile'),
      'file' => 'agileissues.public.inc',
      'page callback' => 'agileissues_vote_up',
      'page arguments' => array(3),
      'access arguments' => array('vote on agile stories'),
    ),
    'agile-public' => array(
      'title' => t('Agile'),
      'file' => 'agileissues.public.inc',
      'page callback' => 'agileissues_public_landing_page',
      'access callback' => 'agileissues_public_access',
      'access arguments' => array('home'),
    ),
    'agile-public/project/%agileissues_project' => array(
      'title' => t('View Agile Project'),
      'file' => 'agileissues.public.inc',
      'page callback' => 'agileissues_project_landing_page',
      'page arguments' => array(2),
      'access callback' => 'agileissues_public_access',
      'access arguments' => array('project', 2),
    ),
    'agile-public/project/%agileissues_project/backlog' => array(
      'title' => t('New Stories'),
      'page callback' => 'agileissues_backlog_landing_page',
      'page arguments' => array(2),
      'file' => 'agileissues.public.inc',
      'access callback' => 'agileissues_public_access',
      'access arguments' => array('backlog', 2),
    ),
    'agile-public/project/%agileissues_project/release-notes' => array(
      'title' => t('Release Notes'),
      'page callback' => 'agileissues_release_notes',
      'page arguments' => array(2),
      'file' => 'agileissues.public.inc',
      'access callback' => 'agileissues_public_access',
      'access arguments' => array('release_notes', 2),
    ),
    'agile-public/project/%agileissues_project/release-notes/%agileissues_sprint' => array(
      'title' => t('Release Notes'),
      'page callback' => 'agileissues_release_notes',
      'page arguments' => array(2, 4),
      'file' => 'agileissues.public.inc',
      'access callback' => 'agileissues_public_access',
      'access arguments' => array('release_notes', 2),
    ),
     'agile-public/project/%agileissues_project/backlog/%agileissues_backlog' => array(
      'title' => t('View Backlog'),
      'page callback' => 'agileissues_backlog_landing_page',
      'page arguments' => array(2, 4),
      'file' => 'agileissues.public.inc',
      'access callback' => 'agileissues_public_access',
      'access arguments' => array('backlog', 2, 4),
     ),
     'agile-public/project/%agileissues_project/sprint' => array(
      'title' => t('Unscheduled Tasks'),
      'file' => 'agileissues.public.inc',
      'page callback' => 'agileissues_sprint_landing_page',
      'page arguments' => array(2),
      'access callback' => 'agileissues_public_access',
      'access arguments' => array('sprint', 2),
     ),
     'agile-public/project/%agileissues_project/sprint/%agileissues_sprint' => array(
      'title' => t('View Sprint'),
      'page callback' => 'agileissues_sprint_landing_page',
      'page arguments' => array(2, 4),
      'file' => 'agileissues.public.inc',
      'access callback' => 'agileissues_public_access',
      'access arguments' => array('sprint', 2, 4),
     ),
     'agile-public/story/%agileissues_story/%ctools_js' => array(
      'title' => t('View Story'),
      'page callback' => 'agileissues_get_content',
      'page arguments' => array('agileissues_story_landing_page', 3, 2),
      'file' => 'agileissues.public.inc',
      'access callback' => 'agileissues_public_access',
      'access arguments' => array('story', 2),
     ),
     'agile-public/task/%agileissues_task/%ctools_js' => array(
      'title' => t('View Task'),
      'file' => 'agileissues.public.inc',
      'page callback' => 'agileissues_get_content',
      'page arguments' => array('agileissues_task_landing_page', 3, 2),
      'access callback' => 'agileissues_public_access',
      'access arguments' => array('task', 2),
     ),
    'agile/search-tips/%ctools_js' => array(
      'title' => 'Agile Search Tips',
      'page callback' => 'agileissues_get_content',
      'page arguments' => array('agileissues_search_tips_page', 2),
      'file' => 'agileissues.pages.inc',
      'type' => MENU_CALLBACK,
      'access callback' => 'agileissues_search_access',
    ),
    'system/ajax/autocomplete-relationship' => array(
      'title' => 'Autocomplete Relationship',
      'page callback' => 'agileissues_relationship_autocomplete',
      'access callback' => TRUE,
      'type' => MENU_CALLBACK,
    ),
    'agile' => array(
      'title' => 'Agile Story Management',
      'page callback' => 'agileissues_frontend_page',
      'access callback' => 'agileissues_frontend_access',
      'file' => 'agileissues.pages.inc',
    ),
    'agile/project/%agileissues_project/stats' => array(
      'title' => 'Statistics',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('agileissues_metrics_form', 2),
      'access arguments' => array('view agile statistics'),
      'file' => 'agileissues.pages.inc',
    ),
    'agile/search' => array(
      'title' => 'Search Agile Stories & Tasks',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('agileissues_search_form'),
      'access callback' => 'agileissues_search_access',
      'file' => 'agileissues.pages.inc',
    ),
    'agile/search/%' => array(
      'title' => 'Search Agile Stories & Tasks',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('agileissues_search_form', 2),
      'access callback' => 'agileissues_search_access',
      'file' => 'agileissues.pages.inc',
    ),
    'agile/open-request/%ctools_js' => array(
      'title' => 'Create Story',
      'page callback' => 'agileissues_get_modal_form',
      'page arguments' => array('agileissues_story_form', 2),
      'access arguments' => array('open new agile stories'),
      'file' => 'agileissues.pages.inc',
      'type' => MENU_CALLBACK,
    ),
    'agile/story/%agileissues_story/%ctools_js' => array(
      'title' => 'View Story',
      'page callback' => 'agileissues_get_content',
      'page arguments' => array('agileissues_story_page', 3, 2),
      'access callback' => 'agileissues_story_access',
      'access arguments' => array('view', 2),
      'file' => 'agileissues.pages.inc',
      'type' => MENU_CALLBACK,
    ),
    'agile/story/%agileissues_story/reassign-backlog/%ctools_js' => array(
      'title' => 'Change the story\'s backlog',
      'page callback' => 'agileissues_get_modal_form',
      'page arguments' => array('agileissues_story_backlog_form', 4, 2),
      'access arguments' => array('manage internal story data'),
      'file' => 'agileissues.pages.inc',
      'type' => MENU_CALLBACK,
    ),
    'agile/task/%agileissues_task/reassign-sprint/%ctools_js' => array(
      'title' => 'Change the task\'s sprint',
      'page callback' => 'agileissues_get_modal_form',
      'page arguments' => array('agileissues_task_sprint_form', 4, 2),
      'access arguments' => array('manage internal task data'),
      'file' => 'agileissues.pages.inc',
      'type' => MENU_CALLBACK,
    ),
    'agile/backlog/%agileissues_backlog/open' => array(
      'title' => 'Open the backlog for requests',
      'page callback' => 'agileissues_ajax_modify_backlog_status',
      'page arguments' => array(3, 'O'),
      'access arguments' => array('manage agile backlogs'),
      'file' => 'agileissues.pages.inc',
      'type' => MENU_CALLBACK,
    ),
    'agile/backlog/%agileissues_backlog/lock' => array(
      'title' => 'Lock the backlog for release',
      'page callback' => 'agileissues_ajax_modify_backlog_status',
      'page arguments' => array(3, 'L'),
      'access arguments' => array('manage agile backlogs'),
      'file' => 'agileissues.pages.inc',
      'type' => MENU_CALLBACK,
    ),
    'agile/backlog/%agileissues_backlog/complete' => array(
      'title' => 'Mark the backlog as being completed',
      'page callback' => 'agileissues_ajax_modify_backlog_status',
      'page arguments' => array(3, 'C'),
      'access arguments' => array('manage agile backlogs'),
      'file' => 'agileissues.pages.inc',
      'type' => MENU_CALLBACK,
    ),
    'agile/story/%agileissues_story/edit/%ctools_js' => array(
      'title' => 'Edit Story',
      'page callback' => 'agileissues_get_modal_form',
      'page arguments' => array('agileissues_story_form', 4, 2),
      'access callback' => 'agileissues_story_access',
      'access arguments' => array('edit', 2),
      'file' => 'agileissues.pages.inc',
      'type' => MENU_CALLBACK,
    ),
    'agile/story/%agileissues_story/move/%ctools_js' => array(
      'title' => 'Move Story',
      'page callback' => 'agileissues_get_modal_form',
      'page arguments' => array('agileissues_story_move_form', 4, 2),
      'access callback' => 'agileissues_story_access',
      'access arguments' => array('move', 2),
      'file' => 'agileissues.pages.inc',
      'type' => MENU_CALLBACK,
    ),
    'agile/story/%agileissues_story/delete/%ctools_js' => array(
      'title' => 'Delete Story',
      'page callback' => 'agileissues_get_modal_form',
      'page arguments' => array('agileissues_story_delete_form', 4, 2),
      'access callback' => 'agileissues_story_access',
      'access arguments' => array('delete', 2),
      'file' => 'agileissues.pages.inc',
      'type' => MENU_CALLBACK,
    ),
    'agile/story/%agileissues_story/note/%ctools_js' => array(
      'title' => 'Add note to story',
      'page callback' => 'agileissues_get_modal_form',
      'page arguments' => array('agileissues_note_form', 4, 'story', 2),
      'access arguments' => array('add agile story notes'),
      'file' => 'agileissues.pages.inc',
      'type' => MENU_CALLBACK,
    ),
    'agile/story/%agileissues_story/task/%ctools_js' => array(
      'title' => 'Add task to story',
      'page callback' => 'agileissues_get_modal_form',
      'page arguments' => array('agileissues_task_form', 4, NULL, 2),
      'access arguments' => array('open new agile tasks'),
      'file' => 'agileissues.pages.inc',
      'type' => MENU_CALLBACK,
    ),
    'agile/task/%agileissues_task/%ctools_js' => array(
      'title' => 'View',
      'page callback' => 'agileissues_get_content',
      'page arguments' => array('agileissues_task_page', 3, 2),
      'access callback' => 'agileissues_task_access',
      'access arguments' => array('view', 2),
      'file' => 'agileissues.pages.inc',
      'type' => MENU_CALLBACK,
    ),
    'agile/task/%agileissues_task/edit/%ctools_js' => array(
      'title' => 'Edit',
      'page callback' => 'agileissues_get_modal_form',
      'page arguments' => array('agileissues_task_form', 4, 2),
      'access callback' => 'agileissues_task_access',
      'access arguments' => array('edit', 2),
      'file' => 'agileissues.pages.inc',
      'type' => MENU_CALLBACK,
    ),
    'agile/task/%agileissues_task/delete/%ctools_js' => array(
      'title' => 'Edit',
      'page callback' => 'agileissues_get_modal_form',
      'page arguments' => array('agileissues_task_delete_form', 4, 2),
      'access callback' => 'agileissues_task_access',
      'access arguments' => array('delete', 2),
      'file' => 'agileissues.pages.inc',
      'type' => MENU_CALLBACK,
    ),
    'agile/task/%agileissues_task/note/%ctools_js' => array(
      'title' => 'Add note to task',
      'page callback' => 'agileissues_get_modal_form',
      'page arguments' => array('agileissues_note_form', 4, 'task', 2),
      'access arguments' => array('add agile task notes'),
      'file' => 'agileissues.pages.inc',
      'type' => MENU_CALLBACK,
    ),
    'agile/task/%agileissues_task/assign/%ctools_js' => array(
      'title' => 'Assign task to a user',
      'page callback' => 'agileissues_get_modal_form',
      'page arguments' => array('agileissues_task_assign_form', 4, 2),
      'access arguments' => array('manage internal task data'),
      'file' => 'agileissues.pages.inc',
      'type' => MENU_CALLBACK,
    ),
    'agile/my-tasks' => array(
      'title' => 'Assigned Tasks',
      'page callback' => 'agileissues_my_tasks_page',
      'access arguments' => array('own agile tasks'),
      'file' => 'agileissues.pages.inc',
    ),
    'agile/my-stories' => array(
      'title' => 'Assigned Stories',
      'page callback' => 'agileissues_my_stories_page',
      'access arguments' => array('own agile stories'),
      'file' => 'agileissues.pages.inc',
    ),
    'agile/my-reported-stories' => array(
      'title' => 'My Stories',
      'page callback' => 'agileissues_my_reported_stories_page',
      'access arguments' => array('open new agile stories'),
      'file' => 'agileissues.pages.inc',
    ),
    'agile/project/%agileissues_project' => array(
      'title' => 'View Project',
      'page callback' => 'agileissues_project_page',
      'page arguments' => array(2),
      'access callback' => 'agileissues_project_access',
      'access arguments' => array(2),
      'file' => 'agileissues.pages.inc',
    ),
    'agile/project/%agileissues_project/open-request/%ctools_js' => array(
      'title' => 'Create Story',
      'page callback' => 'agileissues_get_modal_form',
      'page arguments' => array('agileissues_story_form', 4, NULL, 2),
      'access arguments' => array('open new agile stories'),
      'file' => 'agileissues.pages.inc',
      'type' => MENU_CALLBACK,
    ),
    'agile/project/%agileissues_project/my-stories' => array(
      'title' => 'Assigned Stories',
      'page callback' => 'agileissues_my_stories_page',
      'page arguments' => array(2),
      'access arguments' => array('own agile stories'),
      'file' => 'agileissues.pages.inc',
    ),
    'agile/project/%agileissues_project/my-tasks' => array(
      'title' => 'Assigned Tasks',
      'page callback' => 'agileissues_my_tasks_page',
      'page arguments' => array(2),
      'access arguments' => array('own agile tasks'),
      'file' => 'agileissues.pages.inc',
    ),
    'agile/project/%agileissues_project/my-reported-stories' => array(
      'title' => 'My Stories',
      'page callback' => 'agileissues_my_reported_stories_page',
      'page arguments' => array(2),
      'access arguments' => array('open new agile stories'),
      'file' => 'agileissues.pages.inc',
    ),
    'agile/project/%agileissues_project/story-management' => array(
      'title' => 'Story Management',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('agileissues_manage_stories_form', 2),
      'access arguments' => array('manage agile stories'),
      'file' => 'agileissues.pages.inc',
    ),
    'agile/project/%agileissues_project/story-management/%agileissues_backlog' => array(
      'title' => 'Story Management',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('agileissues_manage_stories_form', 2, 4),
      'access arguments' => array('manage agile stories'),
      'file' => 'agileissues.pages.inc',
    ),
    'agile/project/%agileissues_project/task-management' => array(
      'title' => 'Task Management',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('agileissues_manage_tasks_form', 2),
      'access arguments' => array('manage agile tasks'),
      'file' => 'agileissues.pages.inc',
    ),
    'agile/project/%agileissues_project/task-management/%agileissues_sprint' => array(
      'title' => 'Task Management',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('agileissues_manage_tasks_form', 2, 4),
      'access arguments' => array('manage agile tasks'),
      'file' => 'agileissues.pages.inc',
    ),
    'admin/content/agile' => array(
      'title' => 'Agile',
      'page callback' => 'agileissues_backend_page',
      'access callback' => 'agileissues_backend_access',
      'file' => 'agileissues.backend.inc',
      'type' => MENU_LOCAL_TASK,
    ),
    'admin/content/agile/project' => array(
      'title' => 'Projects',
      'type' => MENU_NORMAL_ITEM,
      'page callback' => 'agileissues_projects_backend_page',
      'access arguments' => array('manage agile projects'),
      'file' => 'agileissues.backend.inc',
    ),
    'admin/content/agile/project/new' => array(
      'title' => 'New Project',
      'type' => MENU_LOCAL_ACTION,
      'page callback' => 'drupal_get_form',
      'page arguments' => array('agileissues_project_form'),
      'access arguments' => array('manage agile projects'),
      'file' => 'agileissues.backend.inc',
    ),
    'admin/content/agile/project/%agileissues_project' => array(
      'title' => 'Edit Project',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('agileissues_project_form', 4),
      'access arguments' => array('manage agile projects'),
      'file' => 'agileissues.backend.inc',
    ),
    'admin/content/agile/project/%agileissues_project/edit' => array(
      'title' => 'Edit',
      'access arguments' => array('manage agile projects'),
      'file' => 'agileissues.backend.inc',
      'weight' => -10,
      'type' => MENU_DEFAULT_LOCAL_TASK & MENU_LOCAL_TASK
    ),
    'admin/content/agile/project/%agileissues_project/delete' => array(
      'title' => 'Delete',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('agileissues_project_delete_form', 4),
      'access arguments' => array('manage agile projects'),
      'file' => 'agileissues.backend.inc',
      'type' => MENU_LOCAL_TASK,
    ),
    'admin/content/agile/backlog' => array(
      'title' => 'Backlogs',
      'type' => MENU_NORMAL_ITEM,
      'page callback' => 'agileissues_backlogs_backend_page',
      'access arguments' => array('manage agile backlogs'),
      'file' => 'agileissues.backend.inc',
    ),
    'admin/content/agile/backlog/new' => array(
      'title' => 'New Backlog',
      'type' => MENU_LOCAL_ACTION,
      'page callback' => 'drupal_get_form',
      'page arguments' => array('agileissues_backlog_form'),
      'access arguments' => array('manage agile backlogs'),
      'file' => 'agileissues.backend.inc',
    ),
    'admin/content/agile/backlog/%agileissues_backlog' => array(
      'title' => 'Edit Backlog',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('agileissues_backlog_form', 4),
      'access arguments' => array('manage agile backlogs'),
      'file' => 'agileissues.backend.inc',
    ),
    'admin/content/agile/backlog/%agileissues_backlog/edit' => array(
      'title' => 'Edit',
      'access arguments' => array('manage agile backlogs'),
      'file' => 'agileissues.backend.inc',
      'weight' => -10,
      'type' => MENU_DEFAULT_LOCAL_TASK & MENU_LOCAL_TASK
    ),
    'admin/content/agile/backlog/%agileissues_backlog/delete' => array(
      'title' => 'Delete',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('agileissues_backlog_delete_form', 4),
      'access arguments' => array('manage agile backlogs'),
      'file' => 'agileissues.backend.inc',
      'type' => MENU_LOCAL_TASK,
    ),
    'admin/content/agile/sprint' => array(
      'title' => 'Sprints',
      'type' => MENU_NORMAL_ITEM,
      'page callback' => 'agileissues_sprints_backend_page',
      'access arguments' => array('manage agile sprints'),
      'file' => 'agileissues.backend.inc',
    ),
    'admin/content/agile/sprint/new' => array(
      'title' => 'New Sprint',
      'type' => MENU_LOCAL_ACTION,
      'page callback' => 'drupal_get_form',
      'page arguments' => array('agileissues_sprint_form'),
      'access arguments' => array('manage agile sprints'),
      'file' => 'agileissues.backend.inc',
    ),
    'admin/content/agile/sprint/%agileissues_sprint' => array(
      'title' => 'Edit Sprint',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('agileissues_sprint_form', 4),
      'access arguments' => array('manage agile sprints'),
      'file' => 'agileissues.backend.inc',
    ),
    'admin/content/agile/sprint/%agileissues_sprint/edit' => array(
      'title' => 'Edit',
      'access arguments' => array('manage agile sprints'),
      'file' => 'agileissues.backend.inc',
      'weight' => -10,
      'type' => MENU_DEFAULT_LOCAL_TASK & MENU_LOCAL_TASK
    ),
    'admin/content/agile/sprint/%agileissues_sprint/delete' => array(
      'title' => 'Delete',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('agileissues_sprint_delete_form', 4),
      'access arguments' => array('manage agile sprints'),
      'file' => 'agileissues.backend.inc',
      'type' => MENU_LOCAL_TASK,
    ),
  );
  $types = array(
    'projects' => 'project',
    'stories' => 'story',
    'tasks' => 'task',
    'backlogs' => 'backlog',
    'sprints' => 'sprint',
  );
  $menu['admin/structure/agile'] = array(
    'title' => 'Agile Structures',
    'description' => 'Manage the different agile structures',
    'page callback' => 'agileissues_structure_page',
    'access arguments' => array('manage agile issue entities'),
    'file' => 'agileissues.admin.inc',
  );
  foreach ($types as $plural => $singular) {
    $menu['admin/structure/agile/' . $singular] = array(
      'title' => ucwords($plural) . ' Types',
      'page callback' => 'agileissues_entity_bundle_list_page',
      'page arguments' => array($singular),
      'access arguments' => array('manage agile issue entities'),
      'file' => 'agileissues.admin.inc',
    );
    $menu['admin/structure/agile/' . $singular . '/new'] = array(
      'title' => 'Add new  ' . $singular . ' type',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('agileissues_entity_bundle_form', $singular),
      'access arguments' => array('manage agile issue entities'),
      'file' => 'agileissues.admin.inc',
      'type' => MENU_LOCAL_ACTION,
    );
    $menu['admin/structure/agile/' . $singular . '/%agileissues_' . $singular . '_type'] = array(
      'title' => 'Edit',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('agileissues_entity_bundle_form', $singular, 4),
      'access arguments' => array('manage agile issue entities'),
      'file' => 'agileissues.admin.inc',
    );
    $menu['admin/structure/agile/' . $singular . '/%agileissues_' . $singular . '_type/edit'] = array(
      'title' => 'Edit',
      'weight' => -10,
      'access arguments' => array('manage agile issue entities'),
      'file' => 'agileissues.admin.inc',
      'type' => MENU_LOCAL_TASK & MENU_DEFAULT_LOCAL_TASK,
    );
    $menu['admin/structure/agile/' . $singular . '/%agileissues_' . $singular . '_type/delete'] = array(
      'title' => 'Delete',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('agileissues_entity_bundle_delete_form', $singular, 4),
      'access arguments' => array('manage agile issue entities'),
      'file' => 'agileissues.admin.inc',
      'type' => MENU_LOCAL_TASK,
    );
  }
  return $menu;
}

function _agileissues_defaults_entity_info() {
  $bundles = array();
  $types = array(
    'agile_project' => array('table' => 'agileissues_project_types', 'path' => 'admin/structure/agile/project/%agileissues_project_type', 'real' => 'admin/structure/agile/project'),
    'agile_story' => array('table' => 'agileissues_story_types', 'path' => 'admin/structure/agile/story/%agileissues_story_type', 'real' => 'admin/structure/agile/story'),
    'agile_task' => array('table' => 'agileissues_task_types', 'path' => 'admin/structure/agile/task/%agileissues_task_type', 'real' => 'admin/structure/agile/task'),
    'agile_backlog' => array('table' => 'agileissues_backlog_types', 'path' => 'admin/structure/agile/backlog/%agileissues_backlog_type', 'real' => 'admin/structure/agile/backlog'),
    'agile_sprint' => array('table' => 'agileissues_sprint_types', 'path' => 'admin/structure/agile/sprint/%agileissues_sprint_type', 'real' => 'admin/structure/agile/sprint'),
  );
  foreach ($types as $entity => $info) {
    $bundles[$entity] = array();
    $etypes = db_select($info['table'], 'tt')
            ->fields('tt')
            ->execute();
    foreach ($etypes as $row) {
      $bundles[$entity][$row->machine_name] = array(
        'label' => t($row->label),  
        'admin' => array(
          'path' => $info['path'],
          'bundle argument' => 4,
          'real path' => $info['real'] . '/' . $row->machine_name,
          'access arguments' => array('manage agile issue entities'),
        ),
      );
    }
  }
  $info = array();
  $info['agile_project'] = array(
    'label' => t('Agile Project'),
    'controller class' => 'AgileProjectController',
    'base table' => 'agileissues_projects',
    'load hook' => 'agileissues_project_load',
    'uri callback' => 'agileissues_project_uri',
    'fieldable' => TRUE,
    'entity keys' => array(
      'id' => 'id',
      'bundle' => 'type',
      'label' => 'name',
    ),
    'bundle keys' => array(
      'bundle' => 'machine_name',
    ),
    'bundles' => $bundles['agile_project'],
    'view modes' => array(
      'full' => array(
        'label' => t('Full'),
        'custom settings' => FALSE,
      ),
      'teaser' => array(
        'label' => t('Teaser'),
        'custom settings' => FALSE,
      ),
      'public' => array(
        'label' => t('Public'),
        'custom settings' => FALSE,
      ),
      'teaser_public' => array(
        'label' => t('Public Teaser'),
        'custom settings' => FALSE,
      ),
    ),
  );
  $info['agile_story'] = array(
    'label' => t('Agile Story'),
    'controller class' => 'AgileStoryController',
    'base table' => 'agileissues_stories',
    'load hook' => 'agileissues_story_load',
    'uri callback' => 'agileissues_story_uri',
    'fieldable' => TRUE,
    'entity keys' => array(
      'id' => 'id',
      'bundle' => 'type',
      'label' => 'title',
    ),
    'bundle keys' => array(
      'bundle' => 'machine_name',
    ),
    'bundles' => $bundles['agile_story'],
    'view modes' => array(
      'full' => array(
        'label' => t('Full'),
        'custom settings' => FALSE,
      ),
      'teaser' => array(
        'label' => t('Teaser'),
        'custom settings' => FALSE,
      ),
      'public' => array(
        'label' => t('Public'),
        'custom settings' => FALSE,
      ),
      'teaser_public' => array(
        'label' => t('Public Teaser'),
        'custom settings' => FALSE,
      ),
    ),
  );
  $info['agile_backlog'] = array(
    'label' => t('Agile Backlog'),
    'controller class' => 'AgileBacklogController',
    'base table' => 'agileissues_backlogs',
    'load hook' => 'agileissues_backlog_load',
    'uri callback' => 'agileissues_backlog_uri',
    'fieldable' => TRUE,
    'entity keys' => array(
      'id' => 'id',
      'bundle' => 'type',
      'label' => 'name',
    ),
    'bundle keys' => array(
      'bundle' => 'machine_name',
    ),
    'bundles' => $bundles['agile_backlog'],
    'view modes' => array(
      'full' => array(
        'label' => t('Full'),
        'custom settings' => FALSE,
      ),
    ),
  );
  $info['agile_task'] = array(
    'label' => t('Agile Task'),
    'controller class' => 'AgileTaskController',
    'base table' => 'agileissues_tasks',
    'load hook' => 'agileissues_task_load',
    'uri callback' => 'agileissues_task_uri',
    'fieldable' => TRUE,
    'entity keys' => array(
      'id' => 'id',
      'bundle' => 'type',
      'label' => 'title',
    ),
    'bundle keys' => array(
      'bundle' => 'machine_name',
    ),
    'bundles' => $bundles['agile_task'],
    'view modes' => array(
      'full' => array(
        'label' => t('Full'),
        'custom settings' => FALSE,
      ),
      'teaser' => array(
        'label' => t('Teaser'),
        'custom settings' => FALSE,
      ),
      'public' => array(
        'label' => t('Public'),
        'custom settings' => FALSE,
      ),
      'teaser_public' => array(
        'label' => t('Public Teaser'),
        'custom settings' => FALSE,
      ),
    ),
  );
  $info['agile_sprint'] = array(
    'label' => t('Agile Sprint'),
    'access callback' => 'agileissues_sprint_access',
    'controller class' => 'AgileSprintController',
    'base table' => 'agileissues_sprints',
    'load hook' => 'agileissues_sprint_load',
    'uri callback' => 'agileissues_sprint_uri',
    'fieldable' => TRUE,
    'entity keys' => array(
      'id' => 'id',
      'bundle' => 'type',
      'label' => 'name',
    ),
    'bundle keys' => array(
      'bundle' => 'machine_name',
    ),
    'bundles' => $bundles['agile_sprint'],
    'view modes' => array(
      'full' => array(
        'label' => t('Full'),
        'custom settings' => FALSE,
      ),
    ),
  );
  $info['agile_note'] = array(
    'label' => t('Agile Note'),
    'controller class' => 'AgileNoteController',
    'base table' => 'agileissues_notes',
    'load hook' => 'agileissues_note_load',
    'uri callback' => 'agileissues_note_uri',
    'fieldable' => TRUE,
    'entity keys' => array(
      'id' => 'id',
      'label' => 'title',
    ),
    'view modes' => array(
      'full' => array(
        'label' => t('Full'),
        'custom settings' => FALSE,
      ),
      'public' => array(
        'label' => t('Full'),
        'custom settings' => FALSE,
      ),
    ),
  );
  return $info;
}

