(define null?
  (lambda (ls)
    (= ls '())))

(define length
  (lambda (ls)
    (if (null? ls) 0 (+ 1 (length (cdr ls))))))

(define append
  (lambda (ls1 ls2)
    (if (null? ls1) ls2 (cons (car ls1) (append (cdr ls1) ls2)))))

(define map
  (lambda (f ls)
    (if (null? ls)
	ls
	(cons (f (car ls)) (map f (cdr ls))))))

(define map2
  (lambda (f ls1 ls2)
    (if (null? ls1)
	'()
	(cons (f (car ls1) (car ls2)) (map2 f (cdr ls1) (cdr ls2))))))

(define list-ref
  (lambda (ls n)
    (if (= n 0) (car ls) (list-ref (cdr ls) (- n 1)))))

