<?php

class AgileTaskController extends EntityApiController {
  
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    global $user;
    $entity->updated = time();
    if (empty($entity->created)) {
      $entity->created = time();
    }
    if (empty($entity->creator)) {
      $entity->creator = $user->uid;
    }
    if (empty($entity->weight)) {
      $entity->weight = 0;
    }
    $old = NULL;
    if (!empty($entity->id)) {
      $old = agileissues_task_load($entity->id);
    }
    parent::save($entity, $transaction);
    
    if (!empty($old)) {
      module_load_include('internal.inc', 'agileissues');
      _agileissues_register_all_changes('task', $old, $entity);
    }
    return TRUE;
  }
  
  public function delete($ids, \DatabaseTransaction $transaction = NULL) {
    parent::delete($ids, $transaction);
    db_delete('agileissues_relationships')
        ->condition('left_entity_type', 'task')
        ->condition('left_entity_id', $ids)
        ->execute();
      db_delete('agileissues_relationships')
        ->condition('right_entity_type', 'task')
        ->condition('right_entity_id', $ids)
        ->execute();
      db_delete('agileissues_notes')
        ->condition('entity_type', 'task')
        ->condition('entity_id', $ids)
        ->execute();
      db_delete('agileissues_changelog')
        ->condition('entity_type', 'task')
        ->condition('entity_id', $ids)
        ->execute();
  }
  
}