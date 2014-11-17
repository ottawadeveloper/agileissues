<?php

class AgileProjectController extends EntityApiController {
  
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    if (empty($entity->created)) {
      $entity->created = time();
    }
    $entity->updated = time();
    parent::save($entity, $transaction);
  }
  
  public function delete($ids, DatabaseTransaction $transaction = NULL) {
    $rollback = FALSE;
    if (empty($transaction)) {
      $transaction = db_transaction();
      $rollback = TRUE;
    }
    try {
      $stories = db_select('agileissues_stories', 'ais')
        ->fields('ais', array('id'))
        ->condition('project_id', $ids)
        ->execute()
        ->fetchAllKeyed(0, 0);
      entity_get_controller('agile_story')->delete($stories, $transaction);
      $sprints = db_select('agileissues_sprints', 'ais')
        ->fields('ais', array('id'))
        ->condition('project_id', $ids)
        ->execute()
        ->fetchAllKeyed(0, 0);
      entity_get_controller('agile_sprints')->delete($sprints, $transaction);
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