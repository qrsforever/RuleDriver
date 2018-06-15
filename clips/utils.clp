;=================================================================
; date: 2018-06-14 18:55:54
; title: utils
;=================================================================

(deffunction load-file (?file)
	(bind ?pos (str-index "." ?file))
	(if (neq ?pos FALSE)
       then
        (bind ?suffix (sub-string (+ ?pos 1) (length ?file) ?file))
        (switch ?suffix
            (case "clp"
             then
                (load* (resolve-file ?file))
            )
            (case "bat"
             then
                (batch* (resolve-file ?file))
            )
            (default (printout warn "Can load file: " ?file crlf))
        )
	   else
   	   	(printout warn "Cannot handle file: " ?file crlf)
   	   	(printout warn "Can only handle .clp or .bat files!" crlf)
	)
)

(deffunction load-files ($?file-list)
    (bind ?end (length$ $?file-list))
    (loop-for-count (?n 1 ?end)
	   do
	   	(load-file (nth$ ?n $?file-list))
	)
)

;-----------------------------------------------------------------
;	retract time <-- (time (now)) from program
;-----------------------------------------------------------------
(defrule retract-time
    (declare (salience -9999))
    ?f <- (time $?)
  =>
    (facts)
    (retract ?f)
)
