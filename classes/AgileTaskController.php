<?php

class AgileTaskController extends EntityApiController {
  
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    global $user;
    $entity->updated = time();
    if (empty($entity->created)) {
      $entity->created = time();
    }
    if (empty($entity->uid)) {
      $entity->uid = $user->uid;
    }
    if (empty($entity->weight)) {
      $entity->weight = 0;
    }
    return parent::save($entity, $transaction);
  }
  
}