/**
 * @file
 * TinyMCE snippets plugin.
 */
"use strict";

tinymce.PluginManager.add('snippets', function (editor, url) {
  editor.ui.registry.addButton('snippets', {
    icon: 'snippet',
    tooltip: editor.options.get('snippetsTooltip'),
    onAction: function () {
      let config = snippetsTools.getDialogConfig();
      editor.windowManager.open(config);
    },
  });

  editor.ui.registry.addMenuItem('snippets', {
    icon: 'snippet',
    text: editor.options.get('snippetsTooltip'),
    onAction: function () {
      let config = snippetsTools.getDialogConfig();
      editor.windowManager.open(config);
    }
  });
});

/**
 * Wrapper for helper functions.
 */
const snippetsTools = {};

/**
 * Loads available snippets from JS config.
 */
snippetsTools.getSnippets = function () {
  let snippets = {};
  let currentFormat = tinymce.activeEditor.options.get('snippetsCurrentFormat');
  if (currentFormat) {
    snippets = Backdrop.settings.filter.formats[currentFormat]['editorSnippets'];
  }
  return snippets;
}

/**
 * Wraps given markup in an HTML document, adds frontend CSS for preview.
 */
snippetsTools.wrap = function (content) {
  let cssFiles = '';
  if (tinymce.activeEditor.contentCSS.length) {
    let cssItems = tinymce.activeEditor.contentCSS;
    for (let i = 0; i < cssItems.length; i++) {
      cssFiles += '<link rel="stylesheet" href="' + cssItems[i] + '" />';
    }
  }
  return '<!DOCTYPE html><html><head><title>Preview</title>' + cssFiles + '</head><body>' + content + '</body></html>';
}

/**
 * Provides a list of options for the dropdown in the dialog.
 */
snippetsTools.getItemslist = function () {
  let options = [];
  let snippets = snippetsTools.getSnippets();
  for (let item in snippets) {
    options.push({
      value: item,
      text: snippets[item]['label']
    });
  }
  return options;
}

/**
 * Returns configuration for the TinyMCE dialog.
 */
snippetsTools.getDialogConfig = function () {
  let snippets = snippetsTools.getSnippets();
  let dialogConfig = {
    title: tinymce.activeEditor.options.get('snippetsTooltip'),
    size: 'medium',
    initialData: {
      snippetname: 'none',
      preview: snippetsTools.wrap(snippets['none']['snippet'])
    },
    onChange: function (api) {
      let value = api.getData().snippetname;
      let snippets = snippetsTools.getSnippets();
      api.setData({
        preview: snippetsTools.wrap(snippets[value]['snippet']),
      });
      let description = '&nbsp;';
      if (snippets[value]['description']) {
        description = snippets[value]['description'];
      }
      document.getElementById('snippet-description').innerHTML = description;
    },
    onSubmit: function (api) {
      const name = api.getData().snippetname;
      let snippets = snippetsTools.getSnippets();
      if (name != 'none') {
        tinymce.activeEditor.execCommand('mceInsertContent', false, snippets[name]['snippet']);
      }
      api.close();
    },
    body: {
      type: 'panel',
      items: [
        {
          name: 'snippetname',
          type: 'selectbox',
          label: 'Name',
          size: 1,
          items: snippetsTools.getItemslist()
        },
        {
          type: 'htmlpanel',
          html: '<div id="snippet-description"></div><div>&nbsp;</div>'
        },
        {
          name: 'preview',
          type: 'iframe',
          label: 'Preview',
          sandboxed: false,
          streamContent: true
        }
      ]
    },
    buttons: [
      {
        type: 'cancel',
        name: 'closeButton',
        text: 'Cancel'
      },
      {
        type: 'submit',
        name: 'submitButton',
        text: 'Insert',
        buttonType: 'primary'
      }
    ]
  };
  return dialogConfig;
};
