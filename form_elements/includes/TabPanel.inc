<?php

// $Id$

/* @file
 *
 *  Allows forms to dynamically add new tabs to a tabpanel in a form.
 */


class TabPanel {

  /**
   * Creates a number of panels based on how many nodes exist.
   *
   * @param array $form
   * @param DOMNodeList $node_list
   */
  static public function populate(&$form, &$registered_nodes) {
    $i = 0;
    foreach ($registered_nodes as &$registered_node) {
      list($hash, $node) = $registered_node;
      $form['content'][$i] = $form['content'][0];
      $form['content'][$i]['#form_builder'] = $form['#form_builder'];
      $form['content'][$i]['#form_builder']['path'][0] .= '[' . $i + 1 . ']'; // XPath indices start at 1
      $form['content'][$i]['#form_builder']['hash'] = $hash;
      $i++;
    }
  }

  /**
   *
   * @global string $base_url
   * @param array $element
   * @return string
   *    
   */
  static public function theme(&$element) {
    global $base_url;
    $path = drupal_get_path('module', 'islandora_form_builder_elements');
    drupal_add_js($path . '/js/jquery-ui-1.8.7.custom.min.js');
    drupal_add_js($path . '/js/tabpanel_ahah.js');
    drupal_add_css($path . '/css/ui-lightness/jquery-ui-1.8.7.custom.css');
    return '<div id=' . t(implode('-', $element['#array_parents'])) . ' class="action-expand-tab">' . $element['#children'] . '</div>';
  }

  /**
   *
   * @param array $form
   */
  static public function themeEachTab(&$form) {
    $name = $form['#name'];
    $title = $form['#title'];
    // Headers
    $count = 1;
    $output = '<ul>';
    $output .= '<li><a href="#' . $name . '-0">' . $title . '</a><span id="add-' . $name . '" class="ui-icon ui-icon-circle-plus" style="float: left; margin: 0.4em 0.2em 0 0; cursor: pointer;">' . $form[0]["add-tab"]['#id'] . '</span></li>';
    $count = 1;
    while (isset($form[$count])) {
      $output .= '<li><a href="#' . $name . '-' . $count . '">' . $count . '</a><span id="remove-' . $name . '-' . $count . '" class="ui-icon ui-icon-close" style="float: left; margin: 0.4em 0.2em 0 0; cursor: pointer;">' . $form[$count]['remove-tab']['#id'] . '</span></li>';
      $count++;
    }
    $output .= '</ul>';
    // Content
    $count = 0;
    while (isset($form[$count])) {
      $output .= '<div id="' . $name . '-' . $count . '">';
      $output .= drupal_render($form[$count]);
      $output .= '</div>';
      $count++;
    }
    return $output;
  }

  /**
   *
   * @param array $form
   * @param <type> $form_state
   * @return boolean 
   */
  public function process(&$form, &$form_state) {
    $form['#tree'] = TRUE;
    $wrapper = $form['#id'] . '-wrapper';
    $this->wrap($form, $wrapper);
    $this->addThemeInfoToContent($form['content'], $form['#title'], $form['#id']);
    $this->createAddTabButton($form['content'], $form['#id'], $wrapper);
    $this->createRemoveTabButtons($form['content'], $form['#id'], $wrapper);
    return $form;
  }

  /**
   * Wraps the tabpanel in a div element, this is used by ahah.
   *
   * @param array $form
   * @param string $wrapper
   */
  private function wrap(&$form, $wrapper) {
    $form['#prefix'] = '<div class="clear-block" id="' . $wrapper . '">';
    $form['#suffix'] = '</div>';
  }

  /**
   *
   * @param array $content
   * @param string $title
   * @param string $id
   */
  private function addThemeInfoToContent(&$content, $title, $id) {
    $content['#title'] = $title;
    $content['#theme'] = 'tabpanel_items';
    $content['#name'] = $id;
  }

  /**
   *
   * @param array $content
   * @param string $id
   * @param string $wrapper
   */
  private function createAddTabButton(&$content, $id, $wrapper) {
    $content[0]['add-tab'] = array(
      '#type' => 'submit',
      '#weight' => 4,
      '#size' => 30,
      '#attributes' => array('style' => 'display:none'), // Hidden, triggered via javascript.
      '#value' => t("$id-add-tab"), // Unique
      '#action' => 'add',
      '#ahah' => array(
        'path' => 'formbuilder/elements/ahah/tabpanel',
        'wrapper' => $wrapper,
        'method' => 'replace',
        'effect' => 'fade'
      )
    );
  }

  /**
   *
   * @param array $content
   * @param string $id
   * @param string $wrapper
   */
  private function createRemoveTabButtons(&$content, $id, $wrapper) {
    $count = 1; // Can't remove the first one.
    while (isset($content[$count])) {
      $content[$count]['remove-tab'] = array(
        '#type' => 'submit',
        '#weight' => 4,
        '#size' => 30,
        '#attributes' => array('style' => 'display:none'), // Hidden, triggered via javascript.
        '#value' => t("$id-remove-tab-$count"), // Unique
        '#action' => 'remove',
        '#tab' => $count,
        '#ahah' => array(
          'path' => 'formbuilder/elements/ahah/tabpanel',
          'wrapper' => $wrapper,
          'method' => 'replace',
          'effect' => 'fade'
        ),
      );
      $count++;
    }
  }

}

/**
 * Gets the form values for $click_button's panel.
 *
 * @param $clicked_button
 *   The button that initiated the ahah callback.
 *
 * @return
 *   An array that contains the values for the panel that contained the 
 *   $clicked_button.
 */
function &ife_get_tabpanel_values($clicked_button) {
  $parents = $clicked_button['#parents'];
  $depth = count($parents) - 2;
  $point = &$_POST;
  for ($i = 0; $i < $depth; $i++) {
    $point = &$point[$parents[$i]];
  }
  return $point;
}

/**
 * Gets the tabpanel's content form element.
 *
 * @param &$form
 *   The cached drupal form.
 * @param $clicked_button
 *   The button that initiated the ahah callback.
 *
 * @return
 *   The tabpanel's content form element.
 */
function &ife_get_tabpanel_content(&$form, $clicked_button) {
  $parents = $clicked_button['#array_parents'];
  $depth = count($parents) - 2; // The button element and the tab its under decreases the depth by 2
  $point = &$form;
  for ($i = 0; $i < $depth; $i++) {
    $point = &$point[$parents[$i]];
  }
  return $point;
}

function &ife_get_tabpanel(&$form, $clicked_button) {
  $parents = $clicked_button['#array_parents'];
  $depth = count($parents) - 3; // The button element and the tab its under decreases the depth by 2
  $point = &$form;
  for ($i = 0; $i < $depth; $i++) {
    $point = &$point[$parents[$i]];
  }
  return $point;
}


/**
 * Calculates the number of tab panels.
 *
 * @param &$content
 *   The tabpanel's content element
 *
 * @return
 *   The number of panels.
 */
function ife_count_tabpanels(&$content) {
  $count = 1;
  while (isset($content[$count]))
    $count++;
  return $count;
}

/**
 * Helper function for ife_remove_tabpanel_from_form
 */
function _ife_remove_tabpanel_from_form(&$form, $clicked_button) {
  $content = &ife_get_tabpanel_content($form, $clicked_button);
  $count = ife_count_tabpanels($content);
  $index = $clicked_button['#tab'];
  for ($i = $index; $i < $count; $i++) {
    if (isset($content[$i + 1])) {
      $content[$i] = $content[$i + 1];
    }
  }
  unset($content[$count - 1]);
}

/**
 * Helper function for ife_remove_tabpanel_values
 */
function _ife_remove_tabpanel_values(&$form, $clicked_button) {
  $content = &ife_get_tabpanel_values($clicked_button);
  $count = count($content);
  $index = $clicked_button['#tab'];
  for ($i = $index; $i < $count; $i++) {
    if (isset($content[$i + 1])) {
      $content[$i] = $content[$i + 1];
    }
  }
  unset($content[$count - 1]);
}

/**
 * Removes a tab panel.
 *
 * @param &$form
 *  Form to remove the tab panel from.
 * @param $clicked_button
 *   The button that initiated the ahah callback.
 */
function ife_remove_tabpanel(&$form, $clicked_button) {
  _ife_remove_tabpanel_from_form($form, $clicked_button);
  _ife_remove_tabpanel_values($form, $clicked_button);
}

/**
 * Removes all tab panels from content except the first panel
 *
 * @param &$content
 *   An array containing the forms repersentation of each panel.
 */
function ife_remove_added_tabpanels(&$content) {
  $count = ife_count_tabpanels($content);
  for ($i = 1; $i < $count; $i++) {
    unset($content[$i]);
  }
}

/**
 * Removes extra tab panels from child tab panel. 
 *
 * Used to remove extra tab panels that might exist in the child tab panels
 * from the copy used to create the new added tab panel.
 *
 * @param &$tabpanel
 *   An array containing the forms repersentation of each panel.
 */
function ife_prune_extra_tabpanels(&$tabpanel) {
  $children = &$tabpanel['children'];
  foreach (element_children($children) as $key) { // Find child tabpanels
    $type = $children[$key]['#type'];
    if (isset($children[$key]['content'])) {
      $content = &$children[$key]['content'];
      ife_remove_added_tabpanels($content);
      ife_prune_extra_tabpanels($content[0]);
    }
  }
}

/**
 * Add a new tab panel.
 *
 * @param &$form
 *   Cached drupal form.
 * @param &$clicked_button
 *   The add button.
 */
function ife_add_tabpanel(&$form, $clicked_button) {
  $content = &ife_get_tabpanel_content($form, $clicked_button);
  $count = ife_count_tabpanels($content);
  $content[$count] = $content[0];
  $tabpanel = &ife_get_tabpanel($form, $clicked_button);
  $content[$count]['#form_builder'] = $tabpanel['#form_builder'];
  $xpath_index = $count + 1;
  $content[$count]['#form_builder']['path'][0] = ''; // New Element, so we can't find it on the path
  unset($content[$count]['#form_builder']['hash']); // New Element, no hash.
  ife_prune_extra_tabpanels($content[$count]);
}

/**
 * Find the modified tabpanel that we will return via ahah.
 *
 * @param &$form
 *   Form to contains the tabpanel
 * @param $clicked_button
 *   The button that initiated the ahah callback.
 */
function ife_find_tabpanel_to_insert(&$form, $clicked_button) {
  $parents = $clicked_button['#array_parents'];
  $depth = count($parents) - 3;
  $point = &$form;
  for ($i = 0; $i < $depth; $i++) {
    $point = &$point[$parents[$i]];
  }
  return $point;
}

/**
 * Checks if a form element in the clicked button that started the ahah 
 * callback.
 *
 * @param &$form
 *   Form element to check.
 */
function is_clicked_button(&$form) {
  if (isset($form['#input']) && $form['#input']) {
    if (isset($form['#executes_submit_callback'])) {
      $name = $form['#name'];
      $value = $form['#value'];
      if (isset($_POST[$name]) && $_POST[$name] == $value) {
        return TRUE;
      }
    }
  }
  return FALSE;
}

/**
 * Searches the form for the button that initiated the ahah callback.
 *
 * @return
 *   The clicked button.
 */
function ife_find_clicked_button($form, $form_state) {
  // Process Form Element
  if ((!empty($form['#type'])) && ($info = _element_info($form['#type']))) {
    $form += $info;
  }
  if (!isset($form['#id'])) {
    $form['#id'] = form_clean_id('edit-' . implode('-', $form['#parents']));
  }
  $process = isset($form['#process']);
  $processed = isset($form['#processed']) && $form['#processed'];
  if ($process && !$processed) {
    foreach ($form['#process'] as $process) {
      if (function_exists($process)) {
        $form = $process($form, isset($edit) ? $edit : NULL, $form_state, NULL);
      }
    }
  }
  // Check if its the Droid we are looking for.
  if (is_clicked_button($form)) {
    return $form;
  }
  // Repeat previous steps for children while letting them know who their parents are.
  foreach (element_children($form) as $key) {
    if (!isset($form[$key]['#tree'])) {
      $form[$key]['#tree'] = $form['#tree'];
    }
    if (!isset($form[$key]['#parents'])) {
      $form[$key]['#parents'] = $form[$key]['#tree'] && $form['#tree'] ? array_merge($form['#parents'], array($key)) : array($key);
      $array_parents = isset($form['#array_parents']) ? $form['#array_parents'] : array();
      $array_parents[] = $key;
      $form[$key]['#array_parents'] = $array_parents;
    }
    // Recurse through all the elements.
    $result = ife_find_clicked_button($form[$key], $form_state);
    if ($result != NULL)
      return $result;
  }
  return NULL;
}

/**
 * Menu hook for ahah callbacks.
 *
 * Depending on the button clicked this will remove a panel or add one.
 */
function islandora_form_builder_elements_ahah_tabpanel() {
  if (!isset($_POST['form_build_id']))
    return;
  $form_state = array('storage' => NULL, 'submitted' => FALSE, 'post' => $_POST);
  $form_build_id = $_POST['form_build_id'];
  if (!$form = form_get_cache($form_build_id, $form_state))
    exit();

  $clicked_button = ife_find_clicked_button($form, $form_state);
  switch ($clicked_button['#action']) {
    case 'remove':
      ife_remove_tabpanel($form, $clicked_button);
      break;
    case 'add':
      ife_add_tabpanel($form, $clicked_button);
      break;
    default:
      exit();
  }

  $form_id = $form['#parameters'][0];
  $form['#post'] = $_POST;
  $form['#programmed'] = FALSE;
  form_set_cache($form_build_id, $form, $form_state);
  $form = form_builder($form_id, $form, $form_state);
  $output = ife_find_tabpanel_to_insert($form, $clicked_button);

  unset($output['#prefix'], $output['#suffix']);
  $javascript = drupal_add_js(NULL, NULL, 'header');
  $settings = call_user_func_array('array_merge_recursive', $javascript['setting']);
  unset($settings['ahah']['']);
  drupal_json(array(
    'status' => TRUE,
    'data' => theme('status_messages') . drupal_render($output),
    'settings' => $settings,
  ));
}

/**
 * Expands the TabPanel form element.
 *
 * @param array $form
 *   Drupal tabpanel form element.
 * @param string $edit
 *   Not used.
 * @param array $form_state
 *   The state of the drupal form.
 *
 * @return array
 *   Modified drupal tabpanel form element.
 */
function islandora_form_builder_elements_process_tabpanel(&$form, $edit, &$form_state) {
  $tab_panel = new TabPanel();
  return $tab_panel->process($form, $form_state);
}

/**
 * Theme hook for a tabpanel
 */
function theme_tabpanel($element) {
  return TabPanel::theme($element);
}

/**
 * Theme hook for the tabpanels contents.
 */
function theme_tabpanel_items($form) {
  return TabPanel::themeEachTab($form);
}