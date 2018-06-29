;=================================================================
; date: 2018-06-04 20:21:39
; title: Unit Test
;=================================================================

(defglobal
    ?*LOG-LEVEL* = (get-debug-level)
    ?*ROOT-DIR* = (get-root-dir)
    ?*CLIPS_DIRS* = (create$ classes rules temlates)
    ?*START-TIME* = (now)
)

(deffunction resolve-file (?file ?ispath)
    (if (eq ?ispath FALSE)
     then
        (foreach ?d ?*CLIPS_DIRS*
            (bind ?path (str-cat ?*ROOT-DIR* "/" ?d "/" ?file))
            (if (open ?path fd)
             then
                (close fd)
                (return ?path)
            )
        )
     else
        (if (open ?file fd)
         then
            (close fd)
            (return ?file)
        )
    )
    (printout error "Not found file: " ?file crlf)
    (return FALSE)
)

(load* (str-cat ?*ROOT-DIR* "/" globals.clp))
(load* (str-cat ?*ROOT-DIR* "/" utils.clp))

(defrule load-files
    (init)
  =>
    (bind ?templfs (get-files 1))
    (bind ?clsesfs (get-files 2))
    (bind ?rulesfs (get-files 3))

    (load-files ?templfs)
    (load-files ?clsesfs)
    (load-files ?rulesfs)

    (assert (load-finished))
)

(defrule init-finished
    (init)
    (load-finished)
  =>
    (reset)
    (if (>= ?*LOG-LEVEL* ?*LOG-LEVEL-TRACE*)
     then
        (printout trace crlf ">>>>>> list globals:" crlf)
        (show-defglobals)
        (printout trace crlf ">>>>>> list defclasses:" crlf)
        (list-defclasses)
        (printout trace crlf ">>>>>> list defrules:" crlf)
        (list-defrules)
        (printout trace crlf)
    )
    (seed (integer (time)))
)

;-----------------------------------------------------------------
;    reset: remove all activations, facts, instances, then
;           1. assign globals,
;           2. assert all facts in deffacts
;           3. create all intances in definstances
;           4. set current moudle to the MAIN
;   TODO: call it by program
;-----------------------------------------------------------------
; (reset)
