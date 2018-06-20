
(defrule example
  (or
    (and
      ?fct_t1 <- (time ?year ?month ?day ?hour ?minute ?second)
      (test (= ?year 2018))
      (test (= ?month 06))
      (test (or (= ?day 20) (= ?day 21) (= ?day 22) ))
    )
    (or
      ?ins_0007A895C8A7 <- (object (is-a TempSensor)
        (CurrentTemperature ?CurrentTemperature &:(> ?CurrentTemperature 50))
      )
      ?ins_DC330D799327 <- (object (is-a Light)
        (onOffLight ?onOffLight &:(= ?onOffLight 1))
      )
    )
  )
=>
  (act_control ins_0007A895C7C7 CurrentTemperature 50)
  (act_control ins_DC330D79932A onOffLight 1)
  (act_notify tellYou "Girlfriend Birthday")
  (act_scene array 100 101)
)

