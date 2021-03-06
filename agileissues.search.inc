<?php

function _agileissues_build_search_vector_tips($for_type) {
  $vectors = agileissues_search_vectors($for_type);
  $tips = array();
  foreach ($vectors as $vector => $info) {
    $tips[$vector] = $info['description'];
  }
  return $tips;
}

function agileissues_build_search_parameters($string, array $set = array(), $for = 'story') {
  $params = array(
    'story' => array(
      'id' => array(),
      'created' => array(),
      'updated' => array(),
    ),
    'task' => array(
      'id' => array(),
      'created' => array(),
      'updated' => array(),
      'status' => array(),
    ),
    'project' => array(
      'id' => array(),
    ),
    'backlog' => array(
      'id' => array(),
    ),
    'sprint' => array(
      'id' => array(),
      'status' => array(),
    ),
  ) + $set;
  $components = agileissues_split_search_query($string);
  $sanitized_components = array();
  foreach ($components as $component) {
    $component = check_plain(trim($component));
    if (!empty($component)) {
      $sanitized_components[] = $component;
    }
  }
  $valid_keys = agileissues_search_vectors($for);
  foreach ($sanitized_components as $component) {
    if (substr($component, 0, 1) === '@') {
      $c = explode('=', $component);
      if (isset($valid_keys[$c[0]])) {
        if (!empty($valid_keys[$c[0]]) && !in_array($for, $valid_keys[$c[0]]['applies_to'])) {
          continue;
        }
        if (isset($c[1])) {
          agileissues_process_search_filter($params, $valid_keys[$c[0]], $c[1]);
          continue;
        }
        else {
          agileissues_process_search_filter($params, $valid_keys[$c[0]], TRUE);
          continue;
        }
      }
    }
    agileissues_process_global_search_term($params, $component);
  }
  return $params;
}

function agileissues_process_global_search_term(&$params, $term, $for = 'story') {
  $global = array(
    'story' => array('title'), 
    'task' => array('title'),
  );
  foreach ($global[$for] as $field) {
    $params[$for][$field][] = array(
      'value' => '%' . $term . '%',
      'op' => 'LIKE',
      'type' => 'property',
    );
  }
  $modules = module_implements('agile_global_search');
  foreach ($modules as $module) {
    $hook = $module . '_agile_global_search';
    $hook($params, $term, $for);
  }
}

function agileissues_process_search_filter(&$params, $conf, $value) {
  if (isset($conf['value'])) {
    $value = $conf['value'];
  }
  if (!empty($conf['sanitize'])) {
    if (is_callable($conf['sanitize'])) {
      $args = isset($conf['sanitize arguments']) ? $conf['sanitize arguments'] : array();
      array_unshift($args, $value);
      $value = call_user_func($conf['sanitize'], $args);
    }
    else {
      $value = filter_var($value, $conf['sanitize']);
    }
  }
  if (empty($conf['data_type'])) {
    $conf['data_type'] = AGILE_SEARCH_DATA_TYPE_EXACT;
  }
  switch ($conf['data_type']) {
    case AGILE_SEARCH_DATA_TYPE_GTE:
      $params[$conf['object']][$conf['property']][] = array(
        'value' => $value,
        'op' => '>=',
        'type' => $conf['type'],
        'strict' => $conf['strict'],
      );
      break;
    case AGILE_SEARCH_DATA_TYPE_LTE:
      $params[$conf['object']][$conf['property']][] = array(
        'value' => $value,
        'op' => '<=',
        'type' => $conf['type'],
        'strict' => $conf['strict'],
      );
      break;
    case AGILE_SEARCH_DATA_TYPE_GT:
      $params[$conf['object']][$conf['property']][] = array(
        'value' => $value,
        'op' => '>',
        'type' => $conf['type'],
        'strict' => $conf['strict'],
      );
      break;
    case AGILE_SEARCH_DATA_TYPE_LT:
      $params[$conf['object']][$conf['property']][] = array(
        'value' => $value,
        'op' => '<',
        'type' => $conf['type'],
        'strict' => $conf['strict'],
      );
      break;
    case AGILE_SEARCH_DATA_TYPE_DATE:
      $range = _agileissues_smart_strtime($value, 'both');
      $params[$conf['object']][$conf['property']][] = array(
        'value' => $range['end'],
        'op' => '<=',
        'type' => $conf['type'],
        'strict' => $conf['strict'],
      );
      $params[$conf['object']][$conf['property']][] = array(
        'value' => $range['start'],
        'op' => '>=',
        'type' => $conf['type'],
        'strict' => $conf['strict'],
      );
      break;
    case AGILE_SEARCH_DATA_TYPE_DATE_RANGE_END:
      $params[$conf['object']][$conf['property']][] = array(
        'value' => _agileissues_smart_strtotime($value, 'end'),
        'op' => '<=',
        'type' => $conf['type'],
        'strict' => $conf['strict'],
      );
      break;
    case AGILE_SEARCH_DATA_TYPE_DATE_RANGE_START:
      $params[$conf['object']][$conf['property']][] = array(
        'value' => _agileissues_smart_strtotime($value, 'start'),
        'op' => '>=',
        'type' => $conf['type'],
        'strict' => $conf['strict'],
      );
      break;
    case AGILE_SEARCH_DATA_TYPE_LIKE:
      $params[$conf['object']][$conf['property']][] = array(
        'value' => '%' . $value . '%',
        'op' => 'LIKE',
        'type' => $conf['type'],
        'strict' => $conf['strict'],
      );
      break;
    case AGILE_SEARCH_DATA_TYPE_NOT:
      if (is_array($value)) {
        $params[$conf['object']][$conf['property']][] = array(
          'value' => $value,
          'type' => $conf['type'],
          'strict' => $conf['strict'],
          'op' => 'NOT IN',
        );
        break;
      }
      else {
        $params[$conf['object']][$conf['property']][] = array(
          'value' => $value,
          'type' => $conf['type'],
          'strict' => $conf['strict'],
          'op' => '!=',
        );
        break;
      }
    case AGILE_SEARCH_DATA_TYPE_EXACT:
    default:
      $params[$conf['object']][$conf['property']][] = array(
        'value' => $value,
        'type' => $conf['type'],
        'strict' => $conf['strict'],
      );
      break;
  }
}

function _agileissues_smart_strtime($strtime, $mode) {
  $yearfix = FALSE;
  if (strlen($strtime) === 4) {
    $yearfix = TRUE;
  }
  $pieces = explode(" ", $strtime, 2);
  $time = '';
  $date = $pieces[0];
  if (count($pieces) > 1) {
    $time = $pieces[1];
  }
  $match = array(
    'year' => NULL,
    'month' => NULL,
    'day' => NULL,
    'hour' => NULL,
    'minute' => NULL,
    'second' => NULL,
  );
  if (!empty($date)) {
    $datePieces = count(preg_split('`[/-]`', $date, 3));
    $match['year'] = 'Y';
    if ($datePieces >= 2) {
      $match['month'] = 'm';
    }
    if ($datePieces >= 3) {
      $match['day'] =  'd';
    }
  }
  if (!empty($time)) {
    $timePieces = count(explode(':', $date));
    $match['hour'] = 'G';
    if ($timePieces >= 2) {
      $match['minute'] = 'i';
    }
    if ($timePieces >= 3) {
      $match['second'] = 's';
    }
  }
  if ($yearfix) {
    $strtime .= '-02-02';
  }
  $commonPattern = '';
  if (!empty($match['year'])) {
    $commonPattern .= $match['year'];
    if (!empty($match['month'])) {
      $commonPattern .= '-' . $match['month'];
      if (!empty($match['day'])) {
        $commonPattern .= '-' . $match['day'];
        if (!empty($match['hour'])) {
          $commonPattern .= ' ' . $match['hour'];
          if (!empty($match['minute'])) {
            $commonPattern .= ':' . $match['minute'];
            if (!empty($match['second'])) {
              $commonPattern .= ':' . $match['second'];
            }
          }
        }
      }
    }
  }
  
  $startPattern = $commonPattern;
  $endPattern = $commonPattern;
  if (empty($match['month'])) {
    $startPattern .= '-01-01 00:00:00';
    $endPattern .= '-12-31 23:59:59';
  }
  elseif (empty($match['day'])) {
    $startPattern .= '-01 00:00:00';
    $month = (int) date('m', $time);
    $year = (int) date('Y', $time);
    $maxdays = array(1 => 31, 2 => 28, 3 => 31, 4 => 30, 5 => 31, 6 => 30, 7 => 31, 8 => 31, 9 => 30, 10 => 31, 11 => 30, 12 => 31);
    if ($year % 4 === 0) {
      $maxdays[2]++;
    }
    if ($year % 100 === 0) {
      $maxdays[2]--;
    }
    if ($year % 400 === 0) {
      $maxdays[2]++;
    }
    $endPattern .= '-' . $maxdays[$month] . ' 23:59:59';
  }
  elseif (empty($match['hour'])) {
    $startPattern .= ' 00:00:00';
    $endPattern .= ' 23:59:59';
  }
  elseif (empty($match['minute'])) {
    $startPattern .= ':00:00';
    $endPattern .= ':59:59';
  }
  elseif (empty($match['second'])) {
    $startPattern .= ':00';
    $endPattern .= ':59';
  }
  $actualTime = strtotime($strtime);
  $range = array(
    'start' => date($startPattern, $actualTime),
    'end' => date($endPattern, $actualTime),
  );
  if (isset($range[$mode])) {
    return $range[$mode];
  }
  return $range;
}

function agileissues_split_search_query($query) {
  $components = array();
  $escapeFlags = array("\\");
  $phraseFlags = array("'", '"', "`");
  $separatorFlags = array(" ", "\r" ,"\n", "\t");
  $lookback = '';
  $inPhrase = FALSE;
  $phraseEnder = NULL;
  $buffer = '';
  for ($k = 0; $k < strlen($query); $k++) {
    $current = $query[$k];
    if (in_array($current, $escapeFlags)) {
      $lookback = $current;
      continue;
    }
    if ($inPhrase) {
      if ($current === $phraseEnder) {
        if (empty($lookback)) {
          $inPhrase = FALSE;
          $components[] = $buffer;
          $buffer = '';
          continue;
        }
        $lookback = '';
      }
      if (!empty($lookback)) {
        $buffer .= $lookback;
        $lookback = '';
      }
      $buffer .= $current;
      continue;
    }
    if (in_array($current, $phraseFlags)) {
      if (empty($lookback)) {
        $inPhrase = TRUE;
        $phraseEnder = $current;
        continue;
      }
      $lookback = '';
    }
    if (!empty($lookback)) {
      $buffer .= $lookback;
      $lookback = '';
    }
    if (in_array($current, $separatorFlags)) {
      $components[] = $buffer;
      $buffer = '';
      continue;
    }
    $buffer .= $current;
  }
  if (!empty($buffer)) {
    $components[] = $buffer;
  }
  return $components;
}

function agileissues_execute_search($query, $pass = array(), $for = 'story') {
  $filters = agileissues_build_search_parameters($query, $pass, $for);
  if ($for === 'story') {
    unset($filters['task']);
  }
  $projectIDs = _agileissues_search_entity('agile_project', $filters['project']);
  if (!empty($projectIDs)) {
    $filters['story']['project_id'][] = array(
      'value' => $projectIDs,
      'type' => 'property',
      'strict' => TRUE,
      'op' => 'IN',
    );
  }
  $backlogIDs = _agileissues_search_entity('agile_backlog', $filters['backlog']);
  if (!empty($backlogIDs)) {
    $filters['story']['backlog_id'][] = array(
      'value' => $backlogIDs,
      'type' => 'property',
      'strict' => TRUE,
      'op' => 'IN',
    );
  }
  $sprintIDs = _agileissues_search_entity('agile_sprint', $filters['sprint']);
  if (!empty($sprintIDs)) {
    if ($for == 'task') {
      $filters['task']['sprint_id'][] = array(
        'value' => $sprintIDs,
        'type' => 'property',
        'strict' => TRUE,
        'op' => 'IN',
      );
    }
    else {
      $storyIDs = db_select('agileissues_tasks', 'ait')
        ->condition('sprint_id', $sprintIDs, 'IN')
        ->fields('ait', array('story_id'))
        ->execute()
        ->fetchAllKeyed(0, 0);
      $filters['story']['id'][] = array(
        'value' => $storyIDs,
        'type' => 'property',
        'strict' => TRUE,
        'op' => 'IN',
      );
    }
  }
  $storyIDs = _agileissues_search_entity('agile_story', $filters['story']);
  if ($for === 'task') {
    $filters['story']['id'][] = array(
      'value' => $storyIDs,
      'type' => 'property',
      'strict' => TRUE,
      'op' => 'IN',
    );
  }
  else {
    return array(
      'entity_type' => 'agile_story',
      'ids' => $storyIDs,
    );
  }
  $taskIDs = _agileissues_search_entity('agile_task', $filters['task']);
  return array(
    'entity_type' => 'agile_task',
    'ids' => $taskIDs,
  );
}

function _agileissues_search_entity($entity_type, $filters) {
  $efq = new EntityFieldQuery();
  $efq->entityCondition('entity_type', $entity_type);
  $any = FALSE;
  foreach ($filters as $key => $fields) {
    foreach ($fields as $field) {
      if (!empty($field['strict'])) {
        if (empty($field['op'])) {
          $field['op'] = NULL;
        }
        switch ($field['type']) {
          case 'property':
            $efq->propertyCondition($key, $field['value'], $field['op']);
            $any = TRUE;
            break;
          case 'field':
            $efq->fieldCondition($key, $field['subfield'], $field['value'], $field['op']);
            $any = TRUE;
            break;
        }
      }
    }
  }
  $moreSpecific = FALSE;
  $results = array();
  foreach ($filters as $key => $fields) {
    foreach ($fields as $field) {
      if (empty($field['strict'])) {
        if (empty($field['op'])) {
          $field['op'] = NULL;
        }
        $testEFQ = clone $efq;
        $moreSpecific = TRUE;
        switch ($field['type']) {
          case 'property':
            $testEFQ->propertyCondition($key, $field['value'], $field['op']);
            break;
          case 'field':
            $testEFQ->fieldCondition($key, $field['subfield'], $field['value'], $field['op']);
            break;
        }
        $rs = $testEFQ->execute();
        if (!empty($rs[$entity_type])) {
          foreach (array_keys($rs[$entity_type]) as $id) {
            if (empty($results[$id])) {
              $results[$id] = 0;
            }
            $results[$id]++;
          }
        }
      }
    }
  }
  if ($any && (!$moreSpecific)) {
    $rs = $efq->execute();
    if (!empty($rs[$entity_type])) {
      foreach (array_keys($rs[$entity_type]) as $id) {
        if (empty($results[$id])) {
          $results[$id] = 0;
        }
        $results[$id]++;
      }
    }
  }
  arsort($results);
  return $results;
}