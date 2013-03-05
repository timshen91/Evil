;(define a 1)
;a
;
;(define-syntax my
;  (syntax-rules ()
;                ((y x) x)))
;(my 3)
;
;(define (a) (+ 3 2))
;(a)
(define (fib x)
  (if (= x 0) 0
              (if (= x 1) 1
                          (+ (fib (- x 1)) (fib (- x 2))))))
(fib 20)
