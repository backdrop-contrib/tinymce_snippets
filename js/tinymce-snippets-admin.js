/**
 * Overrides default JavaScript behaviors of the Token UI in the admin form.
 */
(function ($) {

"use strict";

Backdrop.behaviors.tokenInsert = {
  attach: function (context, settings) {
    $('.token-key').attr({
      'tabindex': 0,
      'aria-label': Backdrop.t('Insert this token into your form'),
      'style': 'cursor: pointer'
    });
    $('.token-key').on('click', function (ev) {
      if (tinymce.activeEditor) {
        tinymce.activeEditor.insertContent(ev.target.textContent);
      }
      else {
        let myField = document.getElementById('edit-snippet-value');
        if (!myField) {
          return;
        }
        var startPos = myField.selectionStart;
        var endPos = myField.selectionEnd;
        myField.value = myField.value.substring(0, startPos)
          + ev.target.textContent
          + myField.value.substring(endPos, myField.value.length);
      }
    });
  }

};

})(jQuery);
