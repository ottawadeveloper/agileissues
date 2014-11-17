<?php

class AgileNoteController extends EntityApiController {
  
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    $entity->updated = time();
    if (empty($entity->created)) {
      $entity->created = time();
    }
    return parent::save($entity, $transaction);
  }
  
}