;=================================================================
; date: 2018-07-02 17:32:48
; title: context
;=================================================================

; Rule Context
(defclass RuleContext (is-a Context) (role concrete)
    (slot rule-id (type STRING))
    (slot timeout-ms (type INTEGER) (default ?*RULE-TIMEOUT-MS*))
    (slot retry-count (type INTEGER) (default ?*RULE-RETRY-COUNT*))
    (slot current-try (type INTEGER) (default 0))
    (slot start-time (type INTEGER) (default-dynamic (nth$ 1 (now))))
    (multislot unanswer-list (type STRING))
    (multislot response-rules (type SYMBOL))
)

(defmessage-handler RuleContext init after ()

)

(defmessage-handler RuleContext delete before ()
    (foreach ?act (create$ ?self:unanswer-list)
        (logi "unanswer actions: " ?act)
    )
    (foreach ?rul (create$ ?self:response-rules)
        (logi "undefrule: " ?rul)
        (undefrule ?rul)
    )
    (return TRUE)
)

(defmessage-handler RuleContext try-again(?clock)
    (if (= ?self:current-try ?self:retry-count)
     then
        (return FALSE)
    )
    (bind ?self:current-try (+ ?self:current-try 1))
    (foreach ?act (create$ ?self:unanswer-list)
        (bind ?funcall (explode$ ?act))
        (bind ?cmd (nth$ 1 ?funcall))
        (bind $?args (rest$ ?funcall))
        (send ?self ?cmd (expand$ $?args))
    )
    (bind ?self:start-time ?clock)
    (return TRUE)
)

(defmessage-handler RuleContext action-success (?str)
    (if (stringp ?str)
     then
        (bind ?pos (member$ ?str ?self:unanswer-list))
        (if (neq ?pos FALSE)
         then
            (slot-direct-delete$ unanswer-list ?pos ?pos)
        )
    )
)

(defmessage-handler RuleContext unanswer-count ()
    (return (length$ ?self:unanswer-list))
)

(defmessage-handler RuleContext is-timeout (?clock)
    (return (> ?clock (+ ?self:start-time ?self:timeout-ms)))
)

; Response:
;   (rule-response ins-id success)
(defmessage-handler RuleContext act-control (?id ?slot ?value)
    (bind ?value (number-to-string ?value))
    (if (instance-existp ?id)
     then
        (bind ?rulname (sym-cat "_"?self:rule-id"-response-" ?id))
        (bind ?action-str (format nil "act-control %s %s %s" ?id ?slot ?value))
        (bind ?RHS (str-cat (format nil "(send [%s]" (instance-name ?self))
                    (format nil " action-success \"%s\")"  ?action-str)
                   ))

        (bind ?pos (member$ ?rulname ?self:response-rules))
        (if (eq ?pos FALSE)
         then
            (logd "act_control(" ?id ", " ?slot ", " ?value ")")
            (if (neq (ins-push ?id ?slot ?value) TRUE)
             then
                ; (eval ?RHS)
                (slot-direct-insert$ unanswer-list 1 ?action-str)
                (bind ?clsname (class (symbol-to-instance-name ?id)))
                (bind ?LHS (str-cat (format nil "(object (is-a %s)" ?clsname)
                            (format nil " (ID ?id &:(eq ?id %s))" ?id)
                            (format nil " (%s ?v &:(eq ?v %s)))" ?slot ?value)
                           ))
                (if (make-rule ?rulname ?*SALIENCE-HIGH* ?LHS ?RHS)
                 then
                    (logi "make rule[" ?rulname "] ok!")
                    (slot-direct-insert$ response-rules 1 ?rulname)
                 else
                    (loge "make rule[" ?rulname "] error!")
                )
            )
        )
     else
        (logw "NOT FOUND: " ?id " instance")
    )
    (return FALSE)
)

; Response:
;   (rule-response notify-id success)
(defmessage-handler RuleContext act-notify (?id ?title ?content)
    (bind ?id (number-to-string ?id))
    (if (and (stringp ?title) (stringp ?content))
     then
        (bind ?rulname (sym-cat "_"?self:rule-id"-response-" ?id))
        (bind ?action-str (implode$ (create$ ?id ?title ?content)))
        (bind ?RHS (str-cat "(send [" (instance-name ?self) "] action-success \"" (escape-quote ?action-str) "\")"))

        (bind ?pos (member$ ?rulname ?self:response-rules))
        (if (eq ?pos FALSE)
         then
            (logd "act_notify(" ?id ", " ?title ", " ?content ")")
            (if (neq (txt-push ?id ?title ?content) TRUE)
             then
                ; (eval ?RHS)
                (slot-direct-insert$ unanswer-list 1 ?action-str)
                (bind ?LHS (str-cat "(rule-response "?id" success)"))
                (if (make-rule ?rulname ?*SALIENCE-HIGH* ?LHS ?RHS)
                 then
                    (logi "make rule[" ?rulname "] ok!")
                    (slot-direct-insert$ response-rules 1 ?rulname)
                 else
                    (logi "make rule[" ?rulname "] error!")
                )
            )
        )
     else
        (logw "Parameters is invalid: (" ?id ", " ?title ", " ?content ")")
    )
    (return FALSE)
)

; Response:
;   (rule-response ruleid sucess)
(defmessage-handler RuleContext act-scene ($?ruleid-list)
    (foreach ?rule-id (create$ $?ruleid-list)
        (bind ?ruleid (sym-cat ?rule-id))
        (if (defrule-module ?ruleid)
          then
            (bind ?rulname (sym-cat "_"?self:rule-id"-response-" ?ruleid))
            (bind ?action-str (str-cat "act-scene "?ruleid))
            (bind ?RHS (str-cat "(send [" (instance-name ?self) "] action-success \"" (escape-quote ?action-str) "\")"))
            (bind ?pos (member$ ?rulname ?self:response-rules))
            (if (eq ?pos FALSE)
             then
                (slot-direct-insert$ unanswer-list 1 ?action-str)
                (bind ?LHS (str-cat "(rule-response "?ruleid" success)"))
                (if (make-rule ?rulname ?*SALIENCE-HIGH* ?LHS ?RHS)
                 then
                    (logi "make rule[" ?rulname "] ok!")
                    (slot-direct-insert$ response-rules 1 ?rulname)
                 else
                    (logi "make rule[" ?rulname "] error!")
                )
            )
            (logd "(act-scene " ?ruleid ")")
            (assert (scene ?ruleid))
        else
            (logw "NOT FOUND: scene:" ?ruleid)
        )
    )
)

(deffunction create-rule-context (?rule-id $?args)
    (if (instance-existp ?rule-id)
     then
        (return FALSE)
    )
    (bind ?timeout (nth$ 1 $?args))
    (bind ?trycount (nth$ 2 $?args))
    (if (neq ?timeout nil)
     then
        (if (neq ?trycount  nil)
         then
            (return (make-instance ?rule-id of RuleContext
                     (rule-id ?rule-id)
                     (timeout-ms ?timeout)
                     (retry-count ?trycount)
                    ))
         else
            (return (make-instance ?rule-id of RuleContext
                     (rule-id ?rule-id)
                     (timeout-ms ?timeout)
                    ))
        )
    )
    (return (make-instance ?rule-id of RuleContext (rule-id ?rule-id)))
)

; check rule response
(defrule check-rule-response
    (datetime ?clock $?other)
    ?obj <- (object (is-a RuleContext) (rule-id ?rule-id))
  =>
    (if (= (send ?obj unanswer-count) 0)
     then
        (logi "rule[" ?rule-id "] exec success!")
        (assert (rule-response ?rule-id success))
        (send-message ?*MSG-RULE-RESPONSE* 0)
     else
        (if (send ?obj is-timeout ?clock)
         then
            ; check retry again
            (if (send ?obj try-again ?clock)
             then
                (logw "rule[" ?rule-id "] exec timeout, try again.")
                (return)
             else
                (logw "rule[" ?rule-id "] exec timeout.")
                (assert (rule-response ?rule-id fail))
                (send-message ?*MSG-RULE-RESPONSE* -1)
            )
         else
            ; not timeout, continue
            (return)
        )
    )
    ; finish the rule, release resource
    (if (unmake-instance ?obj)
     then
        (logi "unmake-instance context " ?obj " success.")
     else
        (logw "unmake-instance context " ?obj " fail.")
    )
)
