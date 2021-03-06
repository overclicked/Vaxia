<?php
/**
 * @file
 * vaxia.features.user_role.inc
 */

/**
 * Implements hook_user_default_roles().
 */
function vaxia_user_default_roles() {
  $roles = array();

  // Exported role: administrator-vaxia.
  $roles['administrator-vaxia'] = array(
    'name' => 'administrator-vaxia',
    'weight' => '9',
  );

  // Exported role: art-evaluator.
  $roles['art-evaluator'] = array(
    'name' => 'art-evaluator',
    'weight' => '8',
  );

  // Exported role: char-evaluator.
  $roles['char-evaluator'] = array(
    'name' => 'char-evaluator',
    'weight' => '6',
  );

  // Exported role: item-evaluator.
  $roles['item-evaluator'] = array(
    'name' => 'item-evaluator',
    'weight' => '7',
  );

  // Exported role: player.
  $roles['player'] = array(
    'name' => 'player',
    'weight' => '3',
  );

  // Exported role: storyteller.
  $roles['storyteller'] = array(
    'name' => 'storyteller',
    'weight' => '4',
  );

  // Exported role: storyteller-assistant.
  $roles['storyteller-assistant'] = array(
    'name' => 'storyteller-assistant',
    'weight' => '5',
  );

  return $roles;
}
