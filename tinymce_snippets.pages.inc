<?php
/**
 * @file
 * Form callback and theme for TinyMCE snippets module.
 */

/**
 * Theme callback for the overview form to render a draggable table.
 */
function theme_tinymce_snippets_overview_form_table($variables) {
  $addlink = l('Add snippet', 'admin/config/content/tinymce-snippets/add');

  $header = array(
    array('data' => t('Label')),
    array('data' => t('Description'), 'class' => 'priority-low'),
    array('data' => t('Language')),
    array('data' => t('Weight')),
    array('data' => t('Action')),
  );

  $rows = array();
  $form = $variables['form'];
  foreach (element_children($form['rows']) as $index => $name) {
    $weight_data = $form['rows'][$name];
    $data = $weight_data['weight']['#data'];
    $row_item = array();
    $name = $data['name'];

    $row_item[] = array(
      'data' => theme('label_machine_name__snippet', array(
        'label' => $data['label'],
        'machine_name' => $name,
      )),
    );
    $row_item[] = array(
      'data' => check_plain($data['description']),
      'class' => array('priority-low'),
    );
    $row_item[] = array('data' => $data['langcode']);
    $row_item[] = array(
      'data' => backdrop_render($weight_data['weight']),
    );
    $links = array(
      '#type' => 'operations',
      '#links' => array(
        'edit' => array(
          'title' => t('Edit'),
          'href' => "admin/config/content/tinymce-snippets/$name/edit",
        ),
        'delete' => array(
          'title' => t('Delete'),
          'href' => "admin/config/content/tinymce-snippets/$name/delete",
        ),
      ),
    );
    $row_item[] = backdrop_render($links);

    $rows[] = array(
      'data' => $row_item,
      'class' => array('draggable'),
    );
  }

  $table = theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('id' => 'snippets-table'),
    'empty' => t('No snippets, yet, you may want to <a href="!url">add one</a>.', array(
      '!url' => url('admin/config/content/tinymce-snippets/add'),
    )),
  ));
  backdrop_add_tabledrag('snippets-table', 'order', 'sibling', 'snippet-weight');

  return $addlink . $table;
}

/**
 * Form callback for overview sorting form.
 */
function tinymce_snippets_overview_form(array $form, array &$form_state) {
  $items = config_get('tinymce_snippets.snippets', 'items');
  $rows = array();
  $row_num = 0;
  foreach ($items as $name => $data) {
    $row_item = array(
      'weight' => array(
        '#type' => 'number',
        '#title' => t('Weight'),
        '#title_display' => 'invisible',
        '#default_value' => $row_num,
        '#data' => $data,
        '#attributes' => array('class' => array('snippet-weight')),
      ),
    );
    $rows[$name] = $row_item;
    $row_num++;
  }
  $form['#tree'] = TRUE;
  $form['sortable'] = array(
    '#theme' => 'tinymce_snippets_overview_form_table',
    'rows' => $rows,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save order'),
    '#disabled' => empty($items),
  );

  return $form;
}

/**
 * Submission handler for tinymce_snippets_overview_form.
 */
function tinymce_snippets_overview_form_submit(array $form, array &$form_state) {
  if (empty($form_state['values']['sortable'])) {
    return;
  }
  $rows = $form_state['values']['sortable']['rows'];
  $order = array();
  foreach ($rows as $name => $values) {
    $order[$values['weight']] = $name;
  }
  ksort($order);

  $config = config('tinymce_snippets.snippets');
  $items = $config->get('items');
  $items_reordered = array();
  foreach ($order as $i => $name) {
    $items_reordered[$name] = $items[$name];
  }
  $config->set('items', $items_reordered);
  $config->save();

  backdrop_set_message(t('The snippets order has been saved.'));
}

/**
 * Form callback for individual snippet forms.
 */
function tinymce_snippets_form(array $form, array &$form_state, array $item = array()) {
  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#default_value' => isset($item['label']) ? $item['label'] : '',
    '#required' => TRUE,
  );
  $form['name'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($item['name']) ? $item['name'] : '',
    '#machine_name' => array(
      'exists' => '_tinymce_snippet_name_exists',
      'source' => array('label'),
    ),
  );
  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#default_value' => isset($item['description']) ? $item['description'] : '',
  );
  $form['snippet'] = array(
    '#type' => 'text_format',
    '#title' => t('Snippet'),
    '#resizable' => FALSE,
    '#default_value' => isset($item['snippet']) ? $item['snippet'] : '',
    '#required' => TRUE,
    '#description' => t('This field supports tokens.') . ' ' . theme('token_tree_link'),
  );
  $form['#attached']['js'][] = backdrop_get_path('module', 'tinymce_snippets') .  '/js/tinymce-snippets-admin.js';

  global $language;
  // ... @todo or maybe "translation"?
  if (module_exists('locale')) {
    $language_options = array();
    foreach (language_list(TRUE) as $langcode => $lang) {
      $option_label = $lang->name;
      if (isset($lang->native)) {
        $option_label .= ' (' . $lang->native . ')';
      }
      $language_options[$langcode] = $option_label;
    }
    $form['langcode'] = array(
      '#type' => 'select',
      '#title' => t('Language'),
      '#options' => $language_options,
      '#default_value' => isset($item['langcode']) ? $item['langcode'] : LANGUAGE_NONE,
      '#empty_value' => LANGUAGE_NONE,
    );
  }
  else {
    $form['langcode'] = array(
      '#type' => 'value',
      '#value' => LANGUAGE_NONE,
    );
  }
  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  if (!empty($item)) {
    $form['actions']['delete'] = array(
      '#type' => 'link',
      '#title' => t('Delete'),
      '#href' => "admin/config/content/tinymce-snippets/{$item['name']}/delete",
      '#options' => array(
        'attributes' => array(
          'class' => array(
            'button', 'button-secondary', 'form-delete',
          ),
        ),
      ),
    );
  }
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => 'admin/config/content/tinymce-snippets',
  );

  return $form;
}

/**
 * Validation callback for snippet forms.
 */
function tinymce_snippets_form_validate($form, &$form_state) {
  $snippet = $form_state['values']['snippet']['value'];
  $dom = new DOMDocument();

  libxml_use_internal_errors(TRUE);
  $success = $dom->loadHTML('<html><body>' . $snippet);
  if (!$success) {
    form_set_error('snippet', 'Unable to parse markup');
  }
  $parser_errors = libxml_get_errors();
  libxml_clear_errors();

  if (!empty($parser_errors)) {
    $errors = array();
    foreach ($parser_errors as $parser_error) {
      $errors[] = $parser_error->message . ' (line: ' . $parser_error->line . ').';
    }
    $message = implode('<br>', $errors);
    form_set_error('snippet', $message);
  }

  $xpath = new DOMXpath($dom);
  $invalid_tags = $xpath->query('//style | //script');
  $has_on = $xpath->query('//*[starts-with(name(@*),"on")]');
  if ($invalid_tags->count() || $has_on->count()) {
    form_set_error('snippet', t('Script tags, style tags or on* attributes are invalid in snippets.'));
  }
  $href_tags = $xpath->query('//*[@href]');
  if ($href_tags->count()) {
    foreach ($href_tags as $item) {
      $value = $item->getAttribute('href');
      if ($value != backdrop_strip_dangerous_protocols($value)) {
        form_set_error('snippet', t('Invalid href attribute value %value.', array(
          '%value' => $value,
        )));
      }
    }
  }
}

/**
 * Submission handler for snippet form.
 */
function tinymce_snippets_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $item = array(
    'name' => $values['name'],
    'label' => $values['label'],
    'description' => $values['description'],
    'langcode' => $values['langcode'],
    'snippet' => $values['snippet']['value'],
  );

  $config = config('tinymce_snippets.snippets');
  // @todo may not exist?
  $items = $config->get('items');
  $items[$values['name']] = $item;
  $config->set('items', $items);
  $config->save();
  $form_state['redirect'] = 'admin/config/content/tinymce-snippets';
  backdrop_set_message(t('The snippet %label has been saved.', array(
    '%label' => $values['label'],
  )));
}

/**
 * Form callback for deleting a snippet.
 */
function tinymce_snippets_delete_confirm(array $form, array &$form_state, array $item) {
  $form['snippet'] = array(
    '#type' => 'value',
    '#value' => $item['name'],
  );
  $form['info'] = array(
    '#type' => 'markup',
    '#markup' => '<p>' . t('Delete snippet %snippet.', array(
      '%snippet' => $item['label'],
    )) . '<br>' . t('This action cannot be undone.') . '</p>',
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#attributes' => array(
      'class' => array('button-danger'),
    ),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => 'admin/config/content/tinymce-snippets',
  );
  return $form;
}

/**
 * Submission handler to delete a snippet.
 */
function tinymce_snippets_delete_confirm_submit($form, &$form_state) {
  $config = config('tinymce_snippets.snippets');
  $config->clear('items.' . $form_state['values']['snippet']);
  $config->save();
  $form_state['redirect'] = 'admin/config/content/tinymce-snippets';
  backdrop_set_message('The snippet has been deleted.');
}

/**
 * Helper function for snippet machine name form item.
 */
function _tinymce_snippet_name_exists($name) {
  $existing_items = config_get('tinymce_snippets.snippets', 'items');

  return array_key_exists($name, $existing_items);
}
