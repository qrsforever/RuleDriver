;=================================================================
; date: 2018-06-30 09:54:55
; title: testcase
;=================================================================

; case template as message
(deftemplate testcase
    (slot what (type STRING))
    (slot arg1 (type INTEGER))
    (slot arg2 (type INTEGER))
    (multislot obj)
)

(defclass Door
  (is-a DEVICE)
  (role concrete) (pattern-match reactive) 
  (slot switch (type INTEGER) (allowed-integers 0 1))
)

(defclass Light
  (is-a DEVICE)
  (role concrete) (pattern-match reactive) 
  (slot switch (type INTEGER) (allowed-integers 0 1))
  (slot temprature (type FLOAT) (range -15.0 95.0))
  (slot color (type STRING) (allowed-strings "red" "geen" "blue"))
)

; Test: do control when instance matched
(defrule rul-001 "test1"
    ?ins-DC330D799327 <- (object (is-a Light)
       (ID ?id &:(eq ?id ins-DC330D799327))
       (switch ?switch &:(= ?switch 1))
     )
  =>
    (printout t (act-control ins-0007A895C7C7 switch 1) crlf)
)

; Test: auto generate rules
(defrule rul-002 "test2"
    ?f <- (rul-002)
  =>
    (retract ?f)
    (bind ?act1 (act-control ins-0007A895C7C7 switch 1))
    (bind ?act2 (act-control ins-DC330D799328 temprature 30.0))
    (handle-control-result -1 rul-002 ?act1 ?act2)
)

;-----------------------------------------------------------------
;   Test Case Rule
;-----------------------------------------------------------------

(defrule test-suite
    ?f <- (test-suite init)
  =>
    (assert (testcase (what "rul-001") (arg1 0)))
    (retract ?f)
)

(defrule testcase-handler
    ?f <- (testcase (what ?what) (arg1 ?arg1) (arg2 ?arg2))
  =>
    (retract ?f)
    (printout t "(testcase (what "?what") (arg1 "?arg1") (arg2 "?arg2"))" crlf) 
    (switch ?what
     (case "rul-001"
      then
        (switch ?arg1
         (case 0 ; create instance
          then
            (make-instance ins-DC330D799327 of Light (switch 0))
            (make-instance ins-DC330D799328 of Light (switch 0))
            (make-instance ins-0007A895C7C7 of Door (switch 0))
         )
         (case 1 ; update new value (not trigger rule, instance is wrong)
          then
            (send [ins-DC330D799328] put-switch 1)
         )
         (case 2 ; update new value (trigger the rule)
          then
            (send [ins-DC330D799327] put-switch 1)
         )
         (default ; enter next case
          then
            (assert (testcase (what "rul-002") (arg1 0) (arg2 0)))
            (return TRUE)
         )
        )
        ; goto next case
        (assert (testcase (what ?what) (arg1 (+ ?arg1 1)) (arg2 ?arg2)))
        (agenda)
        (return TRUE)
     )
     (case "rul-002"
      then
        (switch ?arg1
         (case 0 ; trigger rule 002
          then
            (assert (rul-002))
         )
         (case 1
          then
         )
         (default ; enter next case
          then
            (return TRUE)
         )
        )
        ; goto next case
        (assert (testcase (what ?what) (arg1 (+ ?arg1 1)) (arg2 ?arg2)))
        (agenda)
        (return TRUE)
     )
    )
    (printout t "Unkown testcase !!!" crlf) 
    (return FALSE)
)
