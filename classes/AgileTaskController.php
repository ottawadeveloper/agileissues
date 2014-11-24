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
  
}