<?php

function agileissues_entity_bundle_delete_form($form, &$form_state, $bundle_type, $entity) {
  $form['bundle_type'] = array('#type' => 'value', '#value' => $bundle_type);
  $form['bundle'] = array('#type' => 'value', '#value' => $entity);
  return confirm_form($form, t('Are you sure you want to delete this type and all of its children?'), 'admin/structure/agile/' . $bundle_type, t('You will not be able to undo this action!'));
}

function agileissues_entity_bundle_delete_form_submit($form, &$form_state) {
  if (agileissues_delete_bundle($form_state['values']['bundle_type'], $form_state['values']['bundle']->machine_name)) {
    drupal_set_message(t('Bundle deleted successfully'));
    $form_state['redirect'] = 'admin/structure/agile/' . $form_state['values']['bundle_type'];
  }
  else {
    drupal_set_message(t('There was an error while deleting the bundle.'), 'error');
    $form_state['rebuild'] = TRUE;
  }
}

function agileissues_entity_bundle_form($form, &$form_state, $bundle_type, $entity = NULL) {
  if (empty($entity)) {
    $entity = (object) array(
        'machine_name' => '',
        'label' => '',
    );
  }
  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#required' => TRUE,
    '#default_value' => $entity->label,
    '#maxlength' => 255,
  );
  $form['machine_name'] = array(
    '#default_value' => $entity->machine_name,
    '#access' => empty($entity->machine_name),
    '#type' => 'machine_name',
    '#maxlength' => 255,
    '#machine_name' => array(
      'exists' => 'agileissues_' . $bundle_type . '_type_exists',
      'source' => array('label'),
    ),
  );
  $form['bundle_type'] = array(
    '#type' => 'value',
    '#value' => $bundle_type,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

function agileissues_entity_bundle_form_submit($form, &$form_state) {
  $bundle_def = (object) array(
    'machine_name' => $form_state['values']['machine_name'],
    'label' => $form_state['values']['label'],
  );
  if (agileissues_save_bundle($form_state['values']['bundle_type'], $bundle_def)) {
    drupal_set_message(t('Bundle saved successfully!'));
    $form_state['redirect'] = 'admin/structure/agile/' . $form_state['values']['bundle_type'];
  }
  else {
    drupal_est_message(t('There was an error while saving your bundle'), 'error');
    $form_state['rebuild'] = TRUE;
  }
}

function agileissues_entity_bundle_list_page($type) {
  $bundles = agileissues_load_bundles_of_type($type);
  $rows = array();
  foreach ($bundles as $name => $label) {
    $actions = array(
      l(t('edit'), 'admin/structure/agile/' . $type . '/' . $name),
      l(t('delete'), 'admin/structure/agile/' . $type . '/' . $name . '/delete'),
    );
    if (module_exists('field_ui')) {
      $actions[] = l(t('manage fields'), 'admin/structure/agile/' . $type . '/' . $name . '/fields');
      $actions[] = l(t('manage display'), 'admin/structure/agile/' . $type . '/' . $name . '/display');
    }
    $rows[] = array(
      'label' => t($label),
      'actions' => implode(' | ', $actions),
    );
  }
  return theme('table', array(
      'rows' => $rows,
      'header' => array(
          'label' => t('Label'),
          'actions' => t('Actions'),
      ),
      'empty' => t('There are no bundles created yet.'),
  ));
}

function agileissues_structure_page() {
  module_load_include('admin.inc', 'system');
  return system_admin_menu_block_page();
}