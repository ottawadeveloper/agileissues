<?php

class AgileBacklogController extends EntityApiController {
  
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
        ->condition('backlog_id', $ids)
        ->execute()
        ->fetchAllKeyed(0, 0);
      entity_get_controller('agile_story')->delete($stories, $transaction);
      db_update('agileissues_projects')
          ->condition('default_backlog', $ids)
          ->fields(array(
              'default_backlog' => 0,
          ))
          ->execute();
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