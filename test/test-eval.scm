;;; Test evaluator

; Basic definitions
(define a 'aye)
(define b 'bee)
(define c 'cee)

a
b
c

(set! a 'cee)
(set! c 'aye)

a
b
c

(if #t a b)
(if #f a b)
(if a b c)

; Procedures
(define list
  (lambda ls
    ls))

(define abc (list a b c))

abc

; Quasiquote
;`(a ,b)
;``(a ,(b ,b))  ; nested quasiquotes
;`(1 2 3 ,@abc) ; unquote splicing
