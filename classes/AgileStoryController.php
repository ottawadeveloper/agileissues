<?php

class AgileStoryController extends EntityApiController {
  
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    global $user;
    $entity->updated = time();
    if (empty($entity->created)) {
      $entity->created = time();
    }
    if (empty($entity->reporting_uid)) {
      $entity->reporting_uid = $user->uid;
    }
    module_load_include('inc', 'agileissues');
    $entity->priority_score = agileissues_get_story_priority_score($entity);
    return parent::save($entity, $transaction);
  }
  
  public function delete($ids, DatabaseTransaction $transaction = NULL) {
    $rollback = FALSE;
    if (empty($transaction)) {
      $transaction = db_transaction();
      $rollback = TRUE;
    }
    try {
      $tasks = db_select('agileissues_tasks', 'ait')
              ->condition('story_id', $ids)
              ->fields('ait', array('id'))
              ->execute()
              ->fetchAllKeyed(0, 0);
      entity_get_controller('agile_task')->delete($tasks, $transaction);
      parent::delete($ids, $transaction);
    }
    catch (Exception $ex) {
      if ($rollback) {
        $transaction->rollback();
      }
      throw $ex;
    }
  }
  
}