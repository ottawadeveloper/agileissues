<?php

class AgileSprintController extends EntityApiController {
  
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    if (empty($entity->created)) {
      $entity->created = time();
    }
    $entity->updated = time();
    if (!is_numeric($entity->start_date)) {
      $entity->start_date = strtotime($entity->start_date);
    }
    if (!is_numeric($entity->end_date)) {
      $entity->end_date = strtotime($entity->end_date);
    }
    parent::save($entity, $transaction);
  }
  
  public function delete($ids, DatabaseTransaction $transaction = NULL) {
    $rollback = FALSE;
    if (empty($transaction)) {
      $transaction = db_transaction();
      $rollback = TRUE;
    }
    try {
      $tasks = db_select('agileissues_tasks', 'ait')
              ->condition('sprint_id', $ids)
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