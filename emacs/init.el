  (require 'package)
  (add-to-list 'package-archives '("melpa" . "https://melpa.org/packages/"))
  (package-initialize)

(unless (package-installed-p 'use-package)
  (package-refresh-contents)
  (package-install 'use-package))
(require 'use-package)

(custom-set-variables
 ;; custom-set-variables was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(custom-safe-themes
   '("0f76f9e0af168197f4798aba5c5ef18e07c926f4e7676b95f2a13771355ce850" "1ea82e39d89b526e2266786886d1f0d3a3fa36c87480fad59d8fab3b03ef576e" default))
 '(package-selected-packages '(evil modus-themes)))
(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 )

(setq evil-want-keybinding nil)
(use-package evil
 :ensure t
 :config (evil-mode 1))

(evil-collection-init)

(use-package dired
  :ensure nil
  :commands (dired dired-jump)
  :bind (("C-x C-j" . dired-jump))
  :custom ((dired-listing-switches "-agho --group-directories-first"))
  :config
  (evil-collection-define-key 'normal 'dired-mode-map
    "h" 'dired-up-directory
    "l" 'dired-find-file))


(use-package evil-easymotion
 :ensure t
 :after evil
 :config
   (evilem-default-keybindings "SPC"))


(use-package evil-surround
 :ensure t
 :after evil
 :config (global-evil-surround-mode 1)
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; remaps the default paste to work as in normal OS ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defun delete-selection-and-paste ()
  (interactive)
  (delete-region (region-beginning) (region-end))
  (yank))


(define-key evil-visual-state-map (kbd "p") 'delete-selection-and-paste)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; keeps the default functionality of paste in vim ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-key evil-visual-state-map (kbd "_ p") 'evil-paste-after)
(define-key evil-visual-state-map (kbd "_ P") 'evil-paste-before)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;(with-eval-after-load 'evil
  ;;(defalias 'forward-evil-word 'forward-evil-symbol))

(setq-default evil-symbol-word-search t)
(defadvice evil-inner-word (around underscore-as-word activate)
  (let ((table (copy-syntax-table (syntax-table))))
    (modify-syntax-entry ?_ "w" table)
    (with-syntax-table table
      ad-do-it)))






(use-package counsel-etags
  :ensure t
  :config
  (add-hook 'prog-mode-hook
            (lambda ()
              (add-hook 'after-save-hook
                        'counsel-etags-virtual-update-tags 'append 'local)))
  (setq counsel-etags-update-interval 180))

(global-set-key (kbd "M-.") 'counsel-etags-find-tag)
(global-set-key (kbd "M-,") 'pop-tag-mark)








;;;;;;;;;;;;;;; MOVE BACKUP FOLDER FROM SOURCE FOLDER ;;;;;;;;;;;;;

;; Set backup directory
(setq backup-directory-alist `(("." . "~/.emacs.d/backups")))

;; Create the backup directory if it doesn't exist
(make-directory "~/.emacs.d/backups" t)

;; Keep only a few old versions of files
(setq version-control t)     ; Use version numbers for backups
(setq delete-old-versions t) ; Automatically delete excess backups
(setq kept-old-versions 6)   ; Number of oldest versions to keep
(setq kept-new-versions 9)   ; Number of newest versions to keep
;;;;;;;;;;;;;;; MOVE BACKUP FOLDER FROM SOURCE FOLDER ;;;;;;;;;;;;;




;;;;;;;;;;;;;;;;;;;;;;;;;; FONTS ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Set default font to Cascadia Code
(set-face-attribute 'default nil
                    :family "Cascadia Code"
                    :height 120) ;; Adjust height as needed
;;;;;;;;;;;;;;;;;;;;;;;;;; FONTS ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;; SET THEME ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(load-theme 'wombat t)
;;;;;;;;;;;;;;;;;;;;;;;;;; SET THEME ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;; no scrollbar no window divider
(window-divider-mode 1)
(setq-default window-divider-default-right-width 0)
(setq-default window-divider-default-bottom-width 0)
(set-face-background 'window-divider "black")
(set-face-foreground 'window-divider "black")
(scroll-bar-mode -1)
;;;;;;;;

;;;; line and columns ;;;;;;;
(column-number-mode t)
;;;; line and columns ;;;;;;;


;;;;;;;;;;;;;;;; ABSOLUTE PATH NAME IN BUFFER ;;;;;;;;;;;;;;;;;;;;;;;;;;;
(setq-default mode-line-buffer-identification
  '(:eval (abbreviate-file-name (or (buffer-file-name) (dired-directory)))))
;;;;;;;;;;;;;;;; ABSOLUTE PATH NAME IN BUFFER ;;;;;;;;;;;;;;;;;;;;;;;;;;;


(global-auto-revert-mode 1)
(global-display-line-numbers-mode 1)
(setq display-line-numbers-type 'relative)

(defun open-shell-in-right-split ()
  "Open `shell' in a vertical split to the right of the current window."
  (interactive)
  ;; Split the window to the right if only one window is present
  (when (one-window-p)
    (split-window-right))
  ;; Move focus to the new window
  (other-window 1)
  ;; Change directory to the current buffer's directory
  (unless (null (buffer-file-name))
    (cd (file-name-directory (buffer-file-name))))
  ;; Open shell or switch to it if already running
  (shell))

;; Bind the function to Ctrl + '
(global-set-key (kbd "C-'") 'open-shell-in-right-split)
