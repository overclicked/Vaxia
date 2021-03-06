<?php
/**
 * @file
 * vaxia.dice.inc - Provides Vaxian dice rolling for Vaxia.
 */

/**
 * Implementation of hook_dice_roller_form(), which returns a form for handling basic style rolls.
 */
function vaxia_dice_roller_form($parent_form, $parent_form_state, $parent_form_id) {
  $form['vaxia_rolls'] = array(
    '#type' => 'fieldset',
    '#theme' => 'vaxia_dice_roller',
    '#prefix' => '<div id="vaxia-dice-roller">',
    '#suffix' => '</div>',
    '#weight' => 10,
    '#tree' => TRUE, // This is important, this allows the deltas below to function.
  );
  // Pass additional data from user profile configuration.
  global $user;
  $this_player = user_load($user->uid);
  $character = '_none';
  $lang = $parent_form['language']['#value'];
  if (isset($parent_form_state['values'])) {
    if(isset($parent_form_state['values']['field_comment_character'][$lang][0]['nid'])) {
      $character = $parent_form_state['values']['field_comment_character'][$lang][0]['nid'];
    }
  }
  elseif(!empty($this_player->field_current_character[$lang][0]['nid'])) {
    $character = $this_player->field_current_character[$lang][0]['nid'];
  }
  if ($character && $character != '_none' ) {
    $character = node_load($character);
    if (isset($character->title) && ($character->type == 'character_sheet')) {
      $dice_size = array(100,77,50,30,20,16,12,10,8,6,4,2);
      $stat = array(
        '-1' => '<none>', 
        'field_life' => 'Life', 
        'field_health' => 'Health', 
        'field_endurance' => 'Endurance', 
        'field_constitution' => 'Constitution', 
        'field_strength' => 'Strength', 
        'field_agility' => 'Agility', 
        'field_dexterity' => 'Dexterity', 
        'field_reflexes' => 'Reflexes', 
        'field_intelligence' => 'Intelligence', 
        'field_awareness' => 'Awareness', 
        'field_spirituality' => 'Spirituality', 
        'field_presence' => 'Presence', 
        'field_charisma' => 'Charisma', 
        'field_appearance' => 'Appearance'
      );
      $skills = array(-1 => '<none>');
      if (isset($character->field_skill[$character->language])) {
        foreach ($character->field_skill[$character->language] as $delta => $skill) {
          $this_skill = field_collection_item_load($skill['value']);
          $skills[$skill['value']] = $this_skill->field_skill_name[$character->language][0]['value'];  
        }
      }
      $items = array(-1 => '<none>');
      if (isset($character->field_items[$character->language])) {
        $stat_set = array('life', 'health', 'endurance', 'constitution', 'strength', 'agility', 'dexterity',
          'reflexes', 'intelligence', 'awareness', 'spirituality', 'presence', 'charisma', 'appearance', 'skill'
        );
        foreach ($character->field_items[$character->language] as $delta => $item) {
          $this_item = node_load($item['nid']);
          $mods = array();
          $item_label = $this_item->title;
          $modifies = $penalties = array();
          $qualifies = TRUE;
          foreach ($stat_set as $stat_name) {
            // Grab the stat mod.
            $field_name = 'field_' . $stat_name;
            $stat_stat = isset($character->{$field_name}[$character->language][0]['value']) ?
              $character->{$field_name}[$character->language][0]['value'] : 0;
            $field_name = 'field_item_' . $stat_name;
            $stat_mod = isset($this_item->{$field_name}[$this_item->language][0]['value']) ?
              $this_item->{$field_name}[$this_item->language][0]['value'] : 0;
            $field_name = 'field_item_required_' . $stat_name;
            $stat_req = isset($this_item->{$field_name}[$this_item->language][0]['value']) ? 
              $this_item->{$field_name}[$this_item->language][0]['value'] : 0;
            $field_name = 'field_item_pen_' . $stat_name;
            $stat_pen = isset($this_item->{$field_name}[$this_item->language][0]['value']) ?
              $this_item->{$field_name}[$this_item->language][0]['value'] : 0;  
            // Once collected, figure out what applies
            if ($stat_mod != 0) {
              $modifies[$stat_name] = $stat_mod;
            }
            if ($stat_pen != 0) {
              $penalties[$stat_name] = $stat_pen;
            }
            if ($stat_stat < $stat_req && $stat_name != 'skill') {
              $qualifies = FALSE;
            }
          } // End stat loop
          // Once gathered, make sure we go through all relevant penalities for mods.      
          if (!$qualifies) {
            // Add all penalities for this item to the modifiers.
            foreach ($stat_set as $stat_name) {
              if (isset($penalties[$stat_name])) {
                $this_modifier = isset($modifies[$stat_name]) ? $modifies[$stat_name] : 0;
                $modifies[$stat_name] = $this_modifier + $penalties[$stat_name];
              }
            }
          }
          $modifies_label = array();
          foreach ($modifies as $label => $value) {
            $modifies_label[] = substr($label, 0, 3) . ':' . $value;
          }
          if (!empty($modifies_label)) {
            $item_label .= ' (' . implode(', ', $modifies_label) . ')';
          }
          $items[$this_item->nid] = $item_label;
        } // End items loop
      }
      for ($i=0;$i<5;$i++) {
        $form['vaxia_rolls']['dice'][$i] = _vaxia_dice_roller_row($dice_size, $stat, $skills);
      }
      $form['vaxia_rolls']['items'] = array(
        '#title' => t('Using items'),
        '#type' => 'select',
        '#options' => $items,
        '#multiple' => TRUE,
      );
      $form['vaxia_rolls']['notes'] = array(
        '#type' => 'hidden',
      );
      return $form;
    }
  }
  $form['vaxia_rolls'] = array(
    '#type' => 'item',
    '#theme' => 'vaxia_dice_roller',
    '#markup' => '',
    '#prefix' => '<div id="vaxia-dice-roller">',
    '#suffix' => '</div>',
  );
  return $form;
}

/**
 * Helper function, create the row of dice rolling options.
 */
function _vaxia_dice_roller_row($dice_size, $stat, $skill) {
  $form['number'] = array(
    '#type' => 'select',
    '#options' => range(0, 10),
  );
  $form['number']['#attributes']['class'][] = 'dice-number';
  $form['size'] = array(
    '#type' => 'select',
    '#options' => array_combine($dice_size, $dice_size),
  );
  $form['size']['#attributes']['class'][] = 'dice-size';
  $form['stat'] = array(
    '#type' => 'select',
    '#options' => $stat,
  );
  $form['stat']['#attributes']['class'][] = 'dice-stat';
  $form['skill'] = array(
    '#type' => 'select',
    '#options' => $skill,
  );
  $form['skill']['#attributes']['class'][] = 'dice-skill';
  return $form;
}

/**
 * Implementation of hook_dice_roller_form_submit(), which returns a form for handling basic style rolls.
 */
function vaxia_dice_roller_form_submit($content, $origin = 'node') {
  switch ($origin) {
    case 'comment':
      if (!empty($content->vaxia_rolls)) {
        $command = '';
        $note = $content->vaxia_rolls['notes'] . '<br>';
        $lang = $content->language;

        // Calculate mods and mod note for stats, skills, etc.
        foreach ($content->vaxia_rolls['dice'] as $delta => $roll) {
          $this_command = $roll['number'] . 'd' . $roll['size'];
  
          // Items apply to all rolls.
          $item_mod_note = '';
          $item_mod = 0;
          if (isset($content->vaxia_rolls['items'])) {
            // If character set
            if (isset($content->field_comment_character[$lang][0]['nid'])) {
              $this_stat = substr($roll['stat'], 6); // Trim "field_" off the top.
              $character = node_load($content->field_comment_character[$lang][0]['nid']);         
              $modifies = $penalties = array();
              $qualifies = FALSE;
              // Gather each item.
              foreach ($content->vaxia_rolls['items'] as $this_item) {
                $this_item = node_load($this_item);
                if (isset($this_item->nid)) {
                  $stat_set = array('life', 'health', 'endurance', 'constitution', 'strength', 'agility', 'dexterity',
                    'reflexes', 'intelligence', 'awareness', 'spirituality', 'presence', 'charisma', 'appearance', 'skill',
                  );
                  $qualifies = TRUE;
                  foreach ($stat_set as $stat_name) {
                    // Grab the stat mod.
                    $field_name = 'field_' . $stat_name;
                    $stat_stat = isset($character->{$field_name}[$character->language][0]['value']) ?
                      $character->{$field_name}[$character->language][0]['value'] : 0;
                    if ($stat_name == 'skill') {
                      if (isset($roll['skill']) && $roll['skill'] != -1) {
                        $skill = field_collection_item_load($roll['skill']); 
                        $stat_stat = isset($skill->field_skill_value[$lang][0]['value']) ?
                          $skill->field_skill_value[$lang][0]['value'] : 0;
                      }
                    }
                    $field_name = 'field_item_' . $stat_name;
                    $stat_mod = isset($this_item->{$field_name}[$this_item->language][0]['value']) ?
                      $this_item->{$field_name}[$this_item->language][0]['value'] : 0;
                    $field_name = 'field_item_required_' . $stat_name;
                    $stat_req = isset($this_item->{$field_name}[$this_item->language][0]['value']) ? 
                      $this_item->{$field_name}[$this_item->language][0]['value'] : 0;
                    $field_name = 'field_item_pen_' . $stat_name;
                    $stat_pen = isset($this_item->{$field_name}[$this_item->language][0]['value']) ?
                      $this_item->{$field_name}[$this_item->language][0]['value'] : 0;
                    // Set modifier from the item stat and stat affected.
                    if ($stat_mod != 0) {
                      $modifies[$stat_name] = $stat_mod;
                    }
                    if ($stat_pen != 0) {
                      $penalties[$stat_name] = $stat_pen;
                    }
                    if ($stat_stat < $stat_req) {
                      $qualifies = FALSE;
                    }
                  } // End stat loop
                  // Once gathered, make sure we go through all relevant penalities for mods.      
                  if (!$qualifies) {
                    // Add all penalities for this item to the modifiers.
                    foreach ($stat_set as $stat_name) {
                      if (isset($penalties[$stat_name])) {
                        $this_modifier = isset($modifies[$stat_name]) ? $modifies[$stat_name] : 0;
                        $modifies[$stat_name] = $this_modifier + $penalties[$stat_name];
                      }
                    }
                  }
                  // Gather skill mods.
                  if (isset($modifies['skill']) && $modifies['skill'] != 0 && isset($roll['skill']) && $roll['skill'] != -1) {
                    $skill_mod = isset($modifies[$this_stat]) ? $modifies[$this_stat] : 0;
                    $modifies[$this_stat] = $skill_mod + $modifies['skill'];
                  }
                  // Set modifier from the item stat and skill affected.
                  if (isset($modifies[$this_stat]) && $modifies[$this_stat] != 0 ) {
                    $item_mod_note .= ' + ' . $this_item->title . ' (' . ($modifies[$this_stat]) . ')';
                    $item_mod += $modifies[$this_stat];
                  }
                  // Add percentage descriptions to mod_note.
                  if (isset($this_item->field_percents[$this_item->language][0]['value'])
                    && !empty($this_item->field_percents[$this_item->language][0]['value'])) {
                    $item_mod_note .= ' + ' . $this_item->title . ' (' .
                      $this_item->field_percents[$this_item->language][0]['value'] . ')';
                  }
                }
              } // End for else per item.
            }
          } // Item information gathered.
  

          $mod_note = '';
          $mod = 0;
          if ($roll['number'] > 0) {
            if (isset($roll['stat']) && $roll['stat'] != -1) {
              if (isset($content->field_comment_character[$lang][0]['nid'])) {
                $character = node_load($content->field_comment_character[$lang][0]['nid']);
                $stat = isset($character->{$roll['stat']}[$lang][0]['value']) ? $character->{$roll['stat']}[$lang][0]['value'] : 0;
                $this_stat = substr($roll['stat'], 6); // Trim "field_" off the top.
                $mod_note .= ' + ' . substr($this_stat, 0, 3) . ' (' . $stat . ')';
                $mod += $stat;
              }
            }
            if (isset($roll['skill']) && $roll['skill'] != -1) {
              $skill = field_collection_item_load($roll['skill']);
              $skill_value = isset($skill->field_skill_value[$lang][0]['value']) ? $skill->field_skill_value[$lang][0]['value'] : 0;
              $skill_name = isset($skill->field_skill_name[$lang][0]['value']) ? $skill->field_skill_name[$lang][0]['value'] : 'skill';
              $skill_desc = isset($skill->field_skill_desc[$lang][0]['value']) ? $skill->field_skill_desc[$lang][0]['value'] : '';
              $mod_note .= ' +  <a title="' . $skill_desc . '" href="#">' . $skill_name . '</a> (' . $skill_value . ')';
              $mod += $skill_value;
            }

            // Add items.
            $mod_note .= $item_mod_note;
            $mod += $item_mod;
            // And add the command to the roll set.
            if ($mod != 0) {
              $note .= 'Roll '. $delta + 1 .' : roll(' . $this_command . ')' . $mod_note . '<br>';
              $command .= $this_command . '+' . $mod .';';
            }
            else {
              $note .= 'Roll '. $delta + 1 .' : roll(' . $this_command . ')<br>';
              $command .= $this_command .';';
            }
          }
        }


      }
      if (!empty($command)) {
        module_load_include('inc', 'dice_roller', 'dice_roller.roller');
        $dids = _dice_roller_parse_roll($command, $note, $content->cid, 'c');
      }
      break;
  }
}
