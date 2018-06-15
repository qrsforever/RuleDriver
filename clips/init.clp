;=================================================================
; date: 2018-06-04 20:21:39
; title: Unit Test
;=================================================================

(defglobal
    ?*ROOT_DIR* = (get-root-dir)
    ?*START-TIME* = (now)
)

(deffunction resolve-file (?file)
    (bind ?fn (str-cat ?*ROOT_DIR* "/" ?file))
    (if (open ?fn fd)
     then
        (close fd)
        (return ?fn)
    )
    (printout error "resolve-file " ?fn " error!" crlf)
    (return FALSE)
)

(load* (resolve-file utils.clp))
(load* (resolve-file globals.clp))

(defrule load-files
    (init)
  =>
    (bind ?clsesfs (get-clses-files))
    (bind ?rulesfs (get-rules-files))
    (load-files ?clsesfs)
    (load-files ?rulesfs)

    (assert (load-finished))
)

(defrule init-finished
    (init)
    (load-finished)
  =>
    (reset)
    (seed (integer (time)))
)

;-----------------------------------------------------------------
;	reset: remove all activations, facts, instances, then
;           1. assign globals,
;           2. assert all facts in deffacts
;           3. create all intances in definstances
;           4. set current moudle to the MAIN
;   TODO: call it by program
;-----------------------------------------------------------------
; (reset)
