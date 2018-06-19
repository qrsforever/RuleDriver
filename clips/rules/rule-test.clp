;=================================================================
; date: 2018-06-15 12:21:08
; title: rule-test
;=================================================================

(defrule rule-time-event
    (declare (salience ?*SALIENCE-NORMAL*))
    ?f <- (time ?Y ?M ?D ?h 12 0)
  =>
    (if (>= ?*LOG-LEVEL* ?*LOG-LEVEL-TRACE*)
      then 
      (facts)
    )
    (printout debug "trigger rule-time-event" crlf)
)
