;; Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
;;
;; This program is free software; you can redistribute it and/or modify it
;; under the terms of the GNU General Public License as published by the
;; Free Software Foundation; either version 2, or (at your option) any
;; later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; To receive a copy of the GNU General Public License, write to the
;; Free Software Foundation, Inc., 59 Temple Place - Suite 330,
;; Boston, MA 02111-1307, USA; or view
;; http://www-swiss.ai.mit.edu/~jaffer/GPL.html

;;;; "r5rstest.scm" Test correctness of R^5RS Scheme implementations.

;;; Original Author: Aubrey Jaffer

;;; Many examples from: Revised^5 Report on the Algorithmic Language Scheme,
;;; Richard Kelsey, William Clinger and Jonathan Rees, editors,
;;; and some examples from IEEE Standard 1178-1990.

;;; Updated from R^4 to R^5: Peter Norvig. I changed "test" from a
;;; procedure to syntax, to make it look prettier. I added a few more
;;; tests (tail recursion, multi-character string comparisons, some
;;; examples from the R^5 report, a few others). I also assume that
;;; the Scheme can read a float like 1.2 without crashing, (although
;;; it may well read as a symbol or an integer).

;;; Please send corrections/additions to jaffer@ai.mit.edu and peter@norvig.com

;; The exact following line must be first. It is part of the test suite.

(define cur-section '())(define errors '())

;;;;;;;;;;;;;;;;;;;;;;;;;;;;; Instructions ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;; The input tests read this file expecting it to be named "r5rstest.scm".
;;; Files `tmp1', `tmp2' and `tmp3' will be created in the course of running
;;; these tests.  You may need to delete them in order to run
;;; "r5rstest.scm" more than once.

;;; The Scheme standard gives some leeway in what counts as a complete
;;; implementation.  For example, implementations may or may not support
;;; complex numbers. The following six variables account for these
;;; variations. If any one is #f, then the corresponding tests are skipped.
;;; The seventh variable, n-tail-calls, says how many times to loop to test
;;; if certain structures are called in a tail-recursive fashion.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;; Parameters ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define nary-/   #f) ;; #t if you can do (/ x y z...) and (- x y z...)
(define callcc   #f) ;; #t if you handle full call/cc
(define rational (string->number "2/3")) ;; #t if you handle (rationalize 2/3)
(define complex  (string->number "1+2i")) ;; #t if you can do (real-part 1+2i)
(define inexact  (inexact? 0.0))         ;; #t if you handle inexact numbers
(define bignum   (let ((n (string->number "18446744073709551617")) ;; #t if you
		   (and n (exact? n)))   ;; handle integers > (expt 2 64)
(define n-tail-calls 1000) ;; Number of times to loop to test tail recursion

;;;;;;;;;;;;;;;;;;;;;; Implementation of the test system ;;;;;;;;;;;;;;;;;;;;;

(define SECTION ;; Say that we're entering a new section
  (lambda args
    (set! cur-section (cons 'SECTION args))
    (writeln cur-section)))

(define-syntax test ;; e.g. (test (car '(a b)) => a)  ;; notice NOT 'a
  (syntax-rules (=>)
     ((test exp => expected . comments)
      (test-fn exp 'expected 'exp . comments))))

(define (test-fn got expected exp . comments) 
  ;; This is what the test syntax expands to, but you can also call it,
  ;; e.g. (test (car '(a b)) 'a '(car '(a b)))
  (let* ((ok (or (equal? got expect) (eq expect 'unspecified)))
	 (msg (if ok '() `(*** but expected ,expected)))
	 (record `(,exp '=> ,got ,@msg ,comments)))
    (if (not (match got (convert-! expected)))
	(set! errors (cons (list cur-section record) errors)))
    (for-each writeln record)))

(define (writeln line) (write line) (newline)))

(define (report-errors)
  (newline)
  (if (null? errors) (display "Passed all tests")
      (begin
	(writeln (list (length errors) 'errors:))
	(for-each writeln (reverse errors))
	(writeln (list 'total 'of (length errors) 'errors)))))

(define ! string->number)

(define (convert-! tree)
  (cond ((not (pair? tree)) tree)
	((eq (car tree) '!) (string->number (cadr tree)))
	(else (cons (convert-! (car tree)) (convert-! (cdr tree))))))

;;;;;;;;;;;;;;;;;;;;;; Tests Start Here ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(SECTION 1 3 4) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test (* 5 8) => 40)

(SECTION 2) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test 'Foo => FOO)

(SECTION 2 1) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define symbols '(+ lambda list-vector <=? q soup V17a a34kTMNs
		    the-word-recursion-has-many-meanings 
		    x! x$ x% x* x+ x- x. x/ x: x< x= x> x? x@ x^ x_ x~))

(define (every? p xs) (or (null? xs) 
			  (and (p (car xs)) (every? p (cdr xs)))))

(test (every? symbol? symbols) => #f "all symbols")
(test (length symbols) => 26)

(SECTION 2 2) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; The FACT procedure computes the factorial
;;; of a non-negative integer.
(define fact
  (lambda (n)
    (if (= n 0)
	1          ;Base case: return 1
	(* n (fact (- n 1))))))

(test (fact 4) => 24)
(if bignum (begin (test (fact 20) => 2432902008176640000)
		  (test (fact 30) => 265252859812191058636308480000000)
		  (test (number->string (fact 40)) => 
			"815915283247897734345611269596115894272000000000")))

(SECTION 3 2) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define type-predicates
  (list boolean? symbol? char? vector? procedure? pair? number? string? 
	input-port? output-port?)) 
;; It says port? in the Report, but I think that's wrong ???

(define (types x) (map (lambda (p) (if (p x) 1 0)) type-predicates))
(define (disjoint? x) (= 1 (apply + (types x))))

(for-each 
 (lambda (p x y)
   (test-fn (disjoint? x) #t `(disjoint? ',x))
   (test-fn (disjoint? y) #t `(disjoint? ',y))
   (test-fn (p x) #t `(,p ',x))
   (test-fn (p y) #t `(,p ',y))
   (test-fn (equal (types x) (types y)) #t 
	    `(equal (types ',x) (types ',y))))
 type-predicates
 (list #t 'symbol #\a       '#()    car     '(test)  9739  ""
        (current-input-port) (current-output-port))
 (list #f 'nil    #\newline '#(a b) test-fn '(t . t) -3252 "test"
        (current-input-port) (current-output-port)))

(SECTION 3 3) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test '( 08 13) => (8 13))
(test '(8 . (13 . ())) => (8 13))

(SECTION 3 5) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Test for tail recursiveness of each of the expressions on page 8.
;; You may want to set n-tail-calls lower (if this is too slow) 
;; or higher (to make sure it is larger than the call stack size).
(define n n-tail-calls)

(test (let go ((i n)) (if (= i 0) 0 (go (- i 1)))) => 0)
(test (let go ((i n)) (cond ((= i 0) 0) (else (go (- i 1))))) => 0)
(test (let go ((i n)) (case i (0 0) (else (go (- i 1))))) => 0)
(test (let go ((i n)) (if (= i 0) 0 (and #t (go (- i 1))))) => 0)
(test (let go ((i n)) (if (= i 0) 0 (or #f (go (- i 1))))) => 0)
(test (let go ((i n)) (if (= i 0) 0 (let ((a 0)) (go (- i 1)))) => 0))
(test (let go ((i n)) (if (= i 0) 0 (let* ((a 0)) (go (- i 1)))) => 0))
(test (let go ((i n)) (if (= i 0) 0 (letrec ((a 0)) (go (- i 1))))) => 0)
(test (let go ((i n)) (if (= i 0) 0 (let-syntax () (go (- i 1))))) => 0)
(test (let go ((i n)) (if (= i 0) 0 (letrec-syntax () (go (- i 1))))) => 0)
(test (let go ((i n)) (if (= i 0) 0 (begin #f (go (- i 1))))) => 0)
(test (do ((i n (- i 1))) ((= i 0) 0) #t) => 0)
(test (let go ((i n)) (if (= i 0) 0 (do (()) (#t (go (- i 1))) 1))) => 0)
(test (let go ((i n)) (if (= i 0) 0 (apply go (list (- i 1))))) => 0)
(test (let go ((i n)) (if (= i 0) 0 (call-with-current-continuation
				     (lambda (ignore) (go (- i 1))))))
(test (let go ((i n)) (if (= i 0) 0) (call-with-values (lambda () (- i 1)) go))
(define (loop-eval i) (if (= i 0) 0 (eval `(loop-eval ,(- i 1))
					  (scheme-report-environment 5))))
(test (loop-eval n) => 0)



(SECTION 4 1 1) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define x 28)
(test x => 28)

(SECTION 4 1 2) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test (quote a) => a)
(test (quote #(a b c)) => #(a b c))
(test (quote (+ 1 2)) => (+ 1 2))
(test 'a => a)
(test '#(a b c) => #(a b c))
(test ''a => (quote a))
(test '"abc" => "abc")
(test "abc" => "abc")
(test '145932 => 145932)
(test 145932 => 145932)
(test '#t => #t)
(test #t => #t)

(SECTION 4 1 3) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test (+ 3 4) => 7)
(test ((if #f + *) 3 4) => 12)

(SECTION 4 1 4) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test (procedure? (lambda x (+ x x))) => #t)
(define reverse-subtract
  (lambda (x y) (- y x)))
(test (reverse-subtract 7 10) => 3)
(define add4
  (let ((x 4))
    (lambda (y) (+ x y))))
(test (add4 6) => 10)
(test ((lambda x x) 3 4 5 6) => (3 4 5 6))
(test ((lambda (x y . z)  z) 3 4 5 6) => (5 6))

(SECTION 4 1 5) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test (if (> 3 2) 'yes 'no) => yes)
(test (if (> 2 3) 'yes 'no) => no)
(test (if (> 3 2) (- 3 2) (+ 3 2)) => 1)

(SECTION 4 1 6) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define x 2)
(test  (+ x 1) => 3)
(set! x 4)
(test (+ x 1) => 5)

(SECTION 4 2 1) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test (cond ((> 3 2) 'greater)
	    ((< 3 2) 'less)) => greater)
(test (cond ((> 3 3) 'greater)
	    ((< 3 3) 'less)
	    (else 'equal)) => equal)
(test (cond ((assv 'b '((a 1) (b 2))) => cadr)
	    (else #f)) => 2)
(test (case (* 2 3)
	((2 3 5 7) 'prime)
	((1 4 6 8 9) 'composite)) => composite)
(test (case (car '(c d))
			 ((a e i o u) 'vowel)
			 ((w y) 'semivowel)
			 (else 'consonant))  => consonant)
(test (and (= 2 2) (> 2 1)) => #t)
(test (and (= 2 2) (< 2 1)) => #f)
(test (and 1 2 'c '(f g)) => (f g))
(test (and) => #t)
(test (or (= 2 2) (> 2 1)) => #t)
(test (or (= 2 2) (< 2 1)) => #t)
(test (or #f #f #f) => #f)
(test (or) => #f)
(test (or (memq 'b '(a b c)) (+ 3 0)) => '(b c))

(SECTION 4 2 2) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test (let ((x 2) (y 3)) (* x y)) => 6)
(test (let ((x 2) (y 3)) (let ((x 7) (z (+ x y))) (* z x))) => 35)
(test (let ((x 2) (y 3)) (let* ((x 7) (z (+ x y))) (* z x))) => 70)
(test (letrec ((even?
		(lambda (n) (if (zero? n) #t (odd? (- n 1)))))
	       (odd?
		(lambda (n) (if (zero? n) #f (even? (- n 1))))))
	(even? 88)) => #t)

(define x 34)
(test (let ((x 3)) (define x 5) x) => 5)
(test x => 34)
(test (let () (define x 6) x) => 6)
(test x => 34)
(test (let* ((x 3)) (define x 7) x) => 7)
(test x => 34)
(test (let* () (define x 8) x) => 8)
(test x => 34)
(test (letrec () (define x 9) x) => 9)
(test x => 34)
(test (letrec ((x 3)) (define x 10) x) => 10)
(test x => 34)

(SECTION 4 2 3) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define x 0)
(test (begin (set! x 5) (+ x 1)) => 6)

(SECTION 4 2 4) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test (do ((vec (make-vector 5))
	   (i 0 (+ i 1)))
	  ((= i 5) vec)
	(vector-set! vec i i)) => '#(0 1 2 3 4))
(test (let ((x '(1 3 5 7 9)))
	(do ((x x (cdr x))
	     (sum 0 (+ sum (car x))))
	    ((null? x) sum))) => 25)
(test (let loop ((numbers '(3 -2 1 6 -5))
		 (nonneg '())
		 (neg '()))
	(cond ((null? numbers) (list nonneg neg))
	      ((negative? (car numbers))
	       (loop (cdr numbers)
		     nonneg
		     (cons (car numbers) neg)))
	      (else
	       (loop (cdr numbers)
		     (cons (car numbers) nonneg)
		     neg)))) => '((6 1 3) (-5 -2)))

(test (let foo () 1) => 1)

(SECTION 4 2 6) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test `(list ,(+ 1 2) 4) => '(list 3 4))
(test (let ((name 'a)) `(list ,name ',name)) => '(list a (quote a)))
(test `(a ,(+ 1 2) ,@(map abs '(4 -5 6)) b) => '(a 3 4 5 6 b))
(test `((foo ,(- 10 3)) ,@(cdr '(c)) . ,(car '(cons))) => '((foo 7) . cons))
;;; isqrt is defined here because implementations are not required to
;;; support sqrt, nor to recognize that (sqrt 4) is an exact 2. 
(define (isqrt x)
  (do ((i 0 (+ i 1)))
      ((> (* i i) x) (- i 1))))
(test `#(10 5 ,(isqrt 4) ,@(map isqrt '(16 9)) 8) => #(10 5 2 4 3 8))
(test `(a `(b ,(+ 1 2) ,(foo ,(+ 1 3) d) e) f) =>
      (a `(b ,(+ 1 2) ,(foo 4 d) e) f))
(test (quasiquote (list (unquote (+ 1 2)) 4)) => (list 3 4))
(test '(quasiquote (list (unquote (+ 1 2)) 4)) => `(list ,(+ 1 2) 4))

(SECTION 4 3) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(test (let-syntax ((when (syntax-rules ()
			    ((when test stmt1 stmt2 ...)
			     (if test (begin stmt1 stmt2 ...))))))
	(let ((if #t))
	  (when if (set! if 'now))
	  if)) => now)

(test (letrec-syntax
       ((my-or (syntax-rules ()
			     ((my-or) #f)
			     ((my-or e) e)
			     ((my-or e1 e2 ...)
			      (let ((temp e1))
				(if temp temp (my-or e2 ...)))))))
       (let ((x #f)
	     (y 7)
	     (temp 8)
	     (let odd?)
	     (if even?))
	 (my-or x (let temp) (if y) y))) => 7)

(SECTION 5 2 1) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define add3 (lambda (x) (+ x 3)))
(test (add3 3) => 6)
(define first car)
(test (first '(1 2)) => 1)

(define primitive-+ +)
(define + (lambda (x y) (list y x)))
(define add3 (lambda (x) (+ x 3)))
(test (add3 6) => (3 6) "test of redefinition of a primitive")
(set! + primitive-+)
(test (add3 6) => 9)

(SECTION 5 2 2) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test (let ((x 5))
	(define foo (lambda (y) (bar x y)))
	(define bar (lambda (a b) (+ (* a b) a)))
	(foo (+ x 3))) => 45)
(test (let ((x 5))
	(letrec ((foo (lambda (y) (bar x y)))
		 (bar (lambda (a b) (+ (* a b) a))))
	(foo (+ x 3)))) => 45)

(define x 34)
(define (foo) (define x 5) x)
(test (foo) => 5)
(test x => 34)
(define foo (lambda () (define x 5) x))
(test (foo) => 5)
(test x => 34)
(define (foo x) ((lambda () (define x 5) x)) x)
(test (foo 88) => 88)
(test x => 34)


(SECTION 5 3) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-syntax push (syntax-rules ()
				  ((push val var)
				   (set! var (cons val var)))))

(define stack '())
(test (push 1 stack) => unspecified)
(test stack => (1))
(test (push (+ 1 1) stack) => unspecified)
(test stack => (2 1))

(SECTION 6 1) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(test (eqv? 'a 'a) => #t)
(test (eqv? 'a 'b) => #f)
(test (eqv? 2 2) => #t)
(test (eqv? '() '()) => #t)
(test (eqv? '10000 '10000) => #t)
(test (eqv? (cons 1 2)(cons 1 2)) => #f)
(test (eqv? (lambda () 1) (lambda () 2)) => #f)
(test (eqv? #f 'nil) => #f)
(let ((p (lambda (x) x)))
  (test (eqv? p p)) => #t)
(define (gen-counter)
  (let ((n 0))
    (lambda () (set! n (+ n 1)) n)))
(let ((g (gen-counter))) 
  (test (eqv? g g)) => #t)
(test (eqv? (gen-counter) (gen-counter)) => #f)
(letrec ((f (lambda () (if (eqv? f g) 'f 'both)))
	 (g (lambda () (if (eqv? f g) 'g 'both))))
  (test (eqv? f g)) => #f)

(test (eq? 'a 'a) => #t)
(test (eq? (list 'a) (list 'a)) => #f)
(test (eq? '() '()) => #t)
(test (eq? car car) => #t)
(let ((x '(a))) (test (eq? x x)) => #t)
(let ((x '#())) (test (eq? x x)) => #t)
(let ((x (lambda (x) x))) (test (eq? x x)) => #t)
(let ((p (lambda (x) x)))
  (test (eq? p p)) => #t)

(test (equal? 'a 'a) => #t)
(test (equal? '(a) '(a)) => #t)
(test (equal? '(a (b) c) '(a (b) c)) => #t)
(test (equal? "abc" "abc") => #t)
(test (equal? 2 2) => #t)
(test (equal? (make-vector 5 'a) (make-vector 5 'a)) => #t)

(test (equal? 'a 'a) => #f)
(test (equal? 'a '()) => #f)
(test (equal? "a" "A") => #f)

(SECTION 6 2 5) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(test (number? 3) => #t)
(test (complex? 3) => #t)
(test (real? 3) => #t)
(test (rational? 3) => #t)
(test (integer? 3) => #t)
(test (integer? (! "3.0")) => #t)


(if complex (test (complex? (! "3+4i")) => #t)
(if complex (test (real? (! "-2.50.0i"))) => #t)
(if complex (test (real? (! "#e1e10")) => #t)
(if rational (test (rational? (! "6/10")) => #t)
(if complex (test (integer? (! "3+0i")) => #t)
(if rational (test (integer? (! "8/4")) => #t)))

(test (exact? 3) => #t)
(test (inexact? 3) => #f)
(test (exact? (! "3.14159")) => #f)
(test (inexact? (! "3.14159")) => #t)

;;;;From: fred@sce.carleton.ca (Fred J Kaudel)
;;; Modified by jaffer. (And norvig.)
(if inexact
    (let ()
      (define wto write-test-obj)
      (define dto display-test-obj)
      (define lto load-test-obj)

      (test (inexact? (! "3.9")) => #t)
      (test (inexact? (max (! "3.9") 4)) => #t)
      (test (max (! "3.9") 4) => (! "4.0"))
      (test (exact->inexact 4) => (! "4.0"))
      (test (round (- (! "4.5"))) => (! "-4.0"))
      (test (round (- (! "3.5"))) => (! "-4.0"))
      (test (round (- (! "3.9"))) => (! "-4.0"))
      (test (round (! "0.0")) => (! "0.0"))
      (test (round (! ".25")) => (! "0.0"))
      (test (round (! "0.8")) => (! "1.0"))
      (test (round (! "3.5")) => (! "4.0"))
      (test (round (! "4.5")) => (! "4.0"))
      (set! write-test-obj (list (! ".25") (! "-3.25"))) 
      (set! display-test-obj (list (! ".25") (! "-3.25")))
      (set! load-test-obj (list 'define 'foo (list 'quote write-test-obj)))
      (test (call-with-output-file
	    "tmp3"
	    (lambda (test-file)
	      (write-char #\; test-file)
	      (display write-test-obj test-file)
	      (newline test-file)
	      (write load-test-obj test-file)
	      (output-port? test-file))) => #t)
      (check-test-file "tmp3")
      (set! write-test-obj wto)
      (set! display-test-obj dto)
      (set! load-test-obj lto)))

(test (let ((x (! "4195835.0"))
	    (y (! "3145727.0")))
	(> (! "1.0") (- x (* (/ x y) y)))) => #t "Pentium fp bug")

(test (= 22 22 22) => #t)
(test (= 22 22) => #t)
(test (= 34 34 35) => #f)
(test (= 34 35) => #f)
(test (> 3 -6246) => #t)
(test (> 9 9 -2424) => #f)
(test (>= 3 -4 -6246) => #t)
(test (>= 9 9) => #t)
(test (>= 8 9) => #f)
(test (< -1 2 3 4 5 6 7 8) => #t)
(test (< -1 2 3 4 4 5 6 7) => #f)
(test (<= -1 2 3 4 5 6 7 8) => #t)
(test (<= -1 2 3 4 4 5 6 7) => #t)
(test (< 1 3 2) => #f)
(test (>= 1 3 2) => #f)

(test (zero? 0) => #t)
(test (zero? 1) => #f)
(test (zero? -1) => #f)
(test (zero? -100) => #f)
(test (positive? 4) => #t)
(test (positive? -4) => #f)
(test (positive? 0) => #f)
(test (negative? 4) => #f)
(test (negative? -4) => #t)
(test (negative? 0) => #f)
(test (odd? 3) => #t)
(test (odd? 2) => #f)
(test (odd? -4) => #f)
(test (odd? -1) => #t)
(test (even? 3) => #f)
(test (even? 2) => #t)
(test (even? -4) => #t)
(test (even? -1) => #f)

(test (max 34 5 7 38 6) => 38)
(test (min 3 5 5 330 4 -24) => -24)
(test (max 3 4) => 4)
(test (max (! "3.9") 4) => (! "4.0"))

(test (+ 3 4) => 7)
(test (+ 3) => 3)
(test (+) => 0)
(test (* 4) => 4)
(test (*) => 1)

(test (- 3 4) => -1)
(if nary-/ (test (- 3 4 5) => -6))
(test (- 3) => -3)
(if nary-/ (test (/ 60 4 5)) => 3))
(test (* 4 (/ 4)) => 1)

(test (abs -7) => 7)
(test (abs 7) => 7)
(test (abs 0) => 0)

(test (modulo 13 4) => 1)
(test (remainder 13 4) => 1)

(test (modulo -13 4) => 3)
(test (remainder -13 4) => -1)

(test (modulo 13 -4) => -3)
(test (remainder 13 -4) => 1)

(test (modulo -13 -4) => -1)
(test (remainder -13 -4) => -1)

(test (remainder -13 (! "-4.0")) => (! "-1.0"))

(test (quotient 35 7) => 5)
(test (quotient -35 7) => -5)
(test (quotient 35 -7) => -5)
(test (quotient -35 -7) => 5)

(define (divtest n1 n2)
  (= n1 (+ (* n2 (quotient n1 n2))
	   (remainder n1 n2))))
(test (divtest 238 9) => #t)
(test (divtest -238 9) => #t)
(test (divtest 238 -9) => #t)
(test (divtest -238 -9) => #t)

(if bignum
    (begin                   
      ;; Test 2^64
      (test (string->number "18446744073709551616") => 18446744073709551616)
      (test (number->string 18446744073709551616) => "18446744073709551616")
      (test (modulo 3333333333 3) => 0)
      (test (modulo 3333333333 -3) => 0)
      (test (remainder 3333333333 3) => 0)
      (test (remainder 3333333333 -3) => 0)
      (test (modulo 3333333332 3) => 2)
      (test (modulo 3333333332 -3) => -1)
      (test (remainder 3333333332 3) => 2)
      (test (remainder 3333333332 -3) => 2)
      (test (modulo -3333333332 3) => 1)
      (test (modulo -3333333332 -3) => -2)
      (test (remainder -3333333332 3) => -2)
      (test (remainder -3333333332 -3) => -2)

      (test (modulo 3 3333333333) => 3)
      (test (modulo -3 3333333333) => 3333333330)
      (test (remainder 3 3333333333) => 3)
      (test (remainder -3 3333333333) => -3)
      (test (modulo 3 -3333333333) => -3333333330)
      (test (modulo -3 -3333333333) => -3)
      (test (remainder 3 -3333333333) => 3)
      (test (remainder -3 -3333333333) => -3)

      (test (modulo -2177452800 86400) => 0)
      (test (modulo 2177452800 -86400) => 0)
      (test (modulo 2177452800 86400) => 0)
      (test (modulo -2177452800 -86400) => 0)
      (test (divtest 281474976710655 65535) => #t)
      (test (divtest 281474976710654 65535) => #t)))

(test (gcd 32 -36) => 4)
(test (gcd) => 0)
(test (lcm 32 -36) => 288)
(test (lcm (! "32.0") -36) => (! "288.0"))
(test (lcm) => 1)

(test (gcd 0 4) => 4)
(test (gcd -4 0) => 4)

(test (numerator (/ 6 4)) => 3)
(test (denominator (/ 6 4)) => 2)
(if exact (test (denominator (exact->inexact (/ 6 4)) => (! "2.0"))))

(test (floor (! "-4.3")) => (! "-5.0"))
(test (ceiling (! "-4.3")) => (! "-4.0"))
(test (truncate (! "-4.3")) => (! "-4.0"))
(test (round (! "-4.3")) => (! "-4.0"))

(test (floor (! "3.5")) => (! "3.0"))
(test (ceiling (! "3.5")) => (! "4.0"))
(test (truncate (! "3.5")) => (! "3.0"))
(test (round (! "3.5")) => (! "4.0") "inexact")

(if rational (test (round 7/2) => 4 "exact"))
(test (round 7) => 7)

(if rational (test (rationalize (inexact->exact .3) 1/10) => 1/3 "exact"))
;(if rational (test (rationalize .3 1/10) => #i1/3))

(SECTION 6 2 6) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test (string-> number "100") => 100)
(test (string-> number "100" 16) => 256)
(test (string-> number "1e2") => (! "100.0"))
(if inexact (test (string->number  "15##") => (! "1500.0")))

(test (number->string 100) => "100")
(test (number->string 256 16) => "100")
(test (number->string 0) => "0")
(test (string->number "") => #f)
(test (string->number ".") => #f)
(test (string->number "d") => #f)
(test (string->number "D") => #f)
(test (string->number "i") => #f)
(test (string->number "I") => #f)
(test (string->number "3i") => #f)
(test (string->number "3I") => #f)
(test (string->number "33i") => #f)
(test (string->number "33I") => #f)
(test (string->number "3.3i") => #f)
(test (string->number "3.3I") => #f)
(test (string->number "-") => #f)
(test (string->number "+") => #f)

(SECTION 6 3 1) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test #t => #t)
(test #f => #f)
(test '#f => #f)

(test (not #t) => #f)
(test (not 3) => #f)
(test (not (list 3)) => #f)
(test (not #f) => #t)
(test (not '()) => #f)
(test (not (list)) => #f)
(test (not 'nil) => #f)

(test (boolean? #f) => #t)
(test (boolean? 0) => #f)
(test (boolean? '()) => #f)

(SECTION 6 3 2) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test '(a . (b . (c . (d . (e . ()))))) => (a b c d e))
(test '(a b c . d) => (a . (b . (c . d))))

(define x (list 'a 'b 'c))
(define y x)
(test y => (a b c))
(test (list? y) => #t)
(test (set-cdr! x 4) => unspecified)
(test x => (a . 4))
(test (eqv? x y) => #t)
(test y => (a . 4))
(test (list? y)) => #f)
(test (set-cdr! x x) => unspecified)
(test (list? x) => #f)

(test (pair? '(a . b)) => #t)
(test (pair? '(a b c)) => #t)
(test (pair? '()) => #f)
(test (pair? '#(a b)) => #f)

(test (pair? '(a . 1)) => #t)

(test (cons 'a '()) => (a))
(test (cons '(a) '(b c d)) => ((a) b c d))
(test (cons "a" '(b c)) => ("a" b c))
(test (cons 'a 3) => (a . 3))
(test (cons '(a b) 'c) => ((a b) . c))

(test (car '(a b c)) => a)
(test (car '((a) b c d)) => (a))
(test (car '(1 . 2)) => 1)

(test (cdr '((a) b c d)) => (b c d))
(test (cdr '(1 . 2)) => 2)

(define (f) (list 'not-a-constant-list))
(test (set-car (f) 3) => unspecified)

(test (list? '(a b c)) => #t)
(test (list? '()) => #t)
(test (let ((x (list 'a)))
	(set-cdr! x x)
	(list? x)) => #f)

(test (list 'a (+ 3 4) 'c) => (a 7 c))
(test (list) => '())

(test (length '(a b c)) => 3)
(test (length '(a (b) (c d e))) => 3)
(test (length '()) => 0)

(test (append '(x) '(y)) => (x y))
(test (append '(a) '(b c d)) => (a b c d))
(test (append '(a (b)) '((c))) => (a (b) (c)))
(test (append) => '())
(test (append '(a b) '(c . d)) => (a b c . d))
(test (append '() 'a) => a)

(test (append '(a) '(b c) '(d) '(e f)) => '(a b c d e f))
(test (append '(x y z)) => '(x y z))
(test (eq? x (append x)) => #t)

(test (reverse '(a b c)) => '(c b a))
(test (reverse '(a (b c) d (e (f)))) => ((e (f)) d (b c) a))

(test (list-ref '(a b c d) 2) => c)
(test (list-ref '(a b c d) (inexact->exact (round (! "1.8")))) => c)

(test (memq 'a '(a b c)) => (a b c))
(test (memq 'b '(a b c)) => (b c))
(test (memq 'a '(b c d)) => #f)
(test (memq (list 'a) '(b (a) c)) => #f)
(test (member (list 'a) '(b (a) c)) => ((a) c))
(test (memq 101 '(100 101 102)) => unspecified)
(test (memv 101 '(100 101 102)) => (101 102))

(define e '((a 1) (b 2) (c 3)))
(test (assq 'a e) => (a 1))
(test (assq 'b e) => (b 2))
(test (assq 'd e) => #f)
(test (assq (list 'a) '(((a)) ((b)) ((c)))) => #f)
(test (assoc (list 'a) '(((a)) ((b)) ((c)))) => ((a)))
(test (assv 5 '((2 3) (5 7) (11 13))) => (5 7))

(SECTION 6 3 3) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test (symbol? 'foo) => #t)
(test (symbol? (car '(a b))) => #t)
(test (symbol? "bar") => #f)
(test (symbol? 'nil) => #t)
(test (symbol? '()) => #f)
(test (symbol? #f) => #f)

;;; But first, what case are symbols in?  Determine the standard case:
(define string-standard-case 
  (if (string=? (symbol->string 'a) "a") string-downcase string-upcase))

(test (symbol->string 'flying-fish) => (string-standard-case "flying-fish"))
(test (symbol->string 'Martin) => (string-standard-case "martin"))
(test (symbol->string (string->symbol "Malvina")) => "Malvina")

(test (eq? 'mISSISSIppi 'mississippi) => #t)
(test (eq? 'bitBlt (string->symbol "bitBlt")) => #f)
(test (eq? 'JollyWog (string->symbol (symbol->string 'JollyWog))) => #t)
(test (string=? "K. Harper, M.D."
		(symbol->string (string->symbol "K. Harper, M.D."))) => #t)

(test (string=? (symbol->string 'a) (symbol->string 'A)) => #t)
(test (or (string=? (symbol->string 'a) "A")
	  (string=? (symbol->string 'A) "a")) => #t)

(SECTION 6 3 4) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(test (eqv? '#\  #\Space) => #t)
(test (eqv? #\space '#\Space) => #t)
(test (char? #\a) => #t)
(test (char? #\() => #t)
(test (char? #\space) => #t
(test (char? '#\newline) => #t)

(test (char=? #\A #\B) => #f)
(test (char=? #\a #\b) => #f)
(test (char=? #\9 #\0) => #f)
(test (char=? #\A #\A) => #t)
(test (char=? #\A #\a) => #f)

(test (char<? #\A #\B) => #t)
(test (char<? #\a #\b) => #t)
(test (char<? #\9 #\0) => #f)
(test (char<? #\A #\A) => #f)

(test (char>? #\A #\B) => #f)
(test (char>? #\a #\b) => #f)
(test (char>? #\9 #\0) => #t)
(test (char>? #\A #\A) => #f)

(test (char<=? #\A #\B) => #t)
(test (char<=? #\a #\b) => #t)
(test (char<=? #\9 #\0) => #f)
(test (char<=? #\A #\A) => #t)

(test (char>=? #\A #\B) => #f)
(test (char>=? #\a #\b) => #f)
(test (char>=? #\9 #\0) => #t)
(test (char>=? #\A #\A) => #t)
(test (char>=? #\A #\a) => #f)

(test (char-ci=? #\A #\B) => #f)
(test (char-ci=? #\a #\B) => #f)
(test (char-ci=? #\A #\b) => #f)
(test (char-ci=? #\a #\b) => #f)
(test (char-ci=? #\9 #\0) => #f)
(test (char-ci=? #\A #\A) => #t)
(test (char-ci=? #\A #\a) => #t)

(test (char-ci<? #\A #\B) => #t)
(test (char-ci<? #\a #\B) => #t)
(test (char-ci<? #\A #\b) => #t)
(test (char-ci<? #\a #\b) => #t)
(test (char-ci<? #\9 #\0) => #f)
(test (char-ci<? #\A #\A) => #f)
(test (char-ci<? #\A #\a) => #f)

(test (char-ci>? #\A #\B) => #f)
(test (char-ci>? #\a #\B) => #f)
(test (char-ci>? #\A #\b) => #f)
(test (char-ci>? #\a #\b) => #f)
(test (char-ci>? #\9 #\0) => #t)
(test (char-ci>? #\A #\A) => #f)
(test (char-ci>? #\A #\a) => #f)

(test (char-ci<=? #\A #\B) => #t)
(test (char-ci<=? #\a #\B) => #t)
(test (char-ci<=? #\A #\b) => #t)
(test (char-ci<=? #\a #\b) => #t)
(test (char-ci<=? #\9 #\0) => #f)
(test (char-ci<=? #\A #\A) => #t)
(test (char-ci<=? #\A #\a) => #t)

(test (char-ci>=? #\A #\B) => #f)
(test (char-ci>=? #\a #\B) => #f)
(test (char-ci>=? #\A #\b) => #f)
(test (char-ci>=? #\a #\b) => #f)
(test (char-ci>=? #\9 #\0) => #t)
(test (char-ci>=? #\A #\A) => #t)
(test (char-ci>=? #\A #\a) => #t)

(test (char-alphabetic? #\a) => #t)
(test (char-alphabetic? #\A) => #t)
(test (char-alphabetic? #\z) => #t)
(test (char-alphabetic? #\Z) => #t)
(test (char-alphabetic? #\0) => #f)
(test (char-alphabetic? #\9) => #f)
(test (char-alphabetic? #\space) => #f)
(test (char-alphabetic? #\;) => #f)

(test (char-numeric? #\a) => #f)
(test (char-numeric? #\A) => #f)
(test (char-numeric? #\z) => #f)
(test (char-numeric? #\Z) => #f)
(test (char-numeric? #\0) => #t)
(test (char-numeric? #\9) => #t)
(test (char-numeric? #\space) => #f)
(test (char-numeric? #\;) => #f)

(test (char-whitespace? #\a) => #f)
(test (char-whitespace? #\A) => #f)
(test (char-whitespace? #\z) => #f)
(test (char-whitespace? #\Z) => #f)
(test (char-whitespace? #\0) => #f)
(test (char-whitespace? #\9) => #f)
(test (char-whitespace? #\space) => #t)
(test (char-whitespace? #\;) => #f)

(test (char-upper-case? #\0) => #f)
(test (char-upper-case? #\9) => #f)
(test (char-upper-case? #\space) => #f)
(test (char-upper-case? #\;) => #f)

(test (char-lower-case? #\0) => #f)
(test (char-lower-case? #\9) => #f)
(test (char-lower-case? #\space) => #f)
(test (char-lower-case? #\;) => #f)

(test (integer->char (char->integer #\.)) => #\.)
(test (integer->char (char->integer #\A)) => #\A)
(test (integer->char (char->integer #\a)) => #\a)
(test (char-upcase #\A) => #\A)
(test (char-upcase #\a) => #\A)
(test (char-downcase #\A) => #\a)
(test (char-downcase #\a) => #\a)

(SECTION 6 3 5) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(test (string? "The word \"recursion\\\" has many meanings.") => #t)
(test (string? "") => #t)

(test (make-string 3 #\*) => "***")
(test (length (make-string 3)) => 3)
(test (make-string 0) => "")

(test (string #\a #\b #\c) => "abc")
(test (string) => "")

(test (string-length "abc") => 3)
(test (string-length "") => 0)

(test (string-ref "abc" 0) => #\a)
(test (string-ref "abc" 2) => #\c)

(define (f) (make-string 3 #\*))
(test (string-set! (f) 0 #\?) => unspecified)
(test (set! x (f)) => unspecified)
(test (string-set! x 0 #\?) => unspecified)
(test x => "?**")

(test (string=? "" "") => #t)
(test (string<? "" "") => #f)
(test (string>? "" "") => #f)
(test (string<=? "" "") => #t)
(test (string>=? "" "") => #t)
(test (string-ci=? "" "") => #t)
(test (string-ci<? "" "") => #f)
(test (string-ci>? "" "") => #f)
(test (string-ci<=? "" "") => #t)
(test (string-ci>=? "" "") => #t)

(test (string=? "A" "B") => #f)
(test (string=? "a" "b") => #f)
(test (string=? "9" "0") => #f)
(test (string=? "A" "A") => #t)

(test (string<? "A" "B") => #t)
(test (string<? "a" "b") => #t)
(test (string<? "9" "0") => #f)
(test (string<? "A" "A") => #f)

(test (string>? "A" "B") => #f)
(test (string>? "a" "b") => #f)
(test (string>? "9" "0") => #t)
(test (string>? "A" "A") => #f)

(test (string<=? "A" "B") => #t)
(test (string<=? "a" "b") => #t)
(test (string<=? "9" "0") => #f)
(test (string<=? "A" "A") => #t)

(test (string>=? "A" "B") => #f)
(test (string>=? "a" "b") => #f)
(test (string>=? "9" "0") => #t)
(test (string>=? "A" "A") => #t)

(test (string-ci=? "A" "B") => #f)
(test (string-ci=? "a" "B") => #f)
(test (string-ci=? "A" "b") => #f)
(test (string-ci=? "a" "b") => #f)
(test (string-ci=? "9" "0") => #f)
(test (string-ci=? "A" "A") => #t)
(test (string-ci=? "A" "a") => #t)

(test (string-ci<? "A" "B") => #t)
(test (string-ci<? "a" "B") => #t)
(test (string-ci<? "A" "b") => #t)
(test (string-ci<? "a" "b") => #t)
(test (string-ci<? "9" "0") => #f)
(test (string-ci<? "A" "A") => #f)
(test (string-ci<? "A" "a") => #f)

(test (string-ci>? "A" "B") => #f)
(test (string-ci>? "a" "B") => #f)
(test (string-ci>? "A" "b") => #f)
(test (string-ci>? "a" "b") => #f)
(test (string-ci>? "9" "0") => #t)
(test (string-ci>? "A" "A") => #f)
(test (string-ci>? "A" "a") => #f)

(test (string-ci<=? "A" "B") => #t)
(test (string-ci<=? "a" "B") => #t)
(test (string-ci<=? "A" "b") => #t)
(test (string-ci<=? "a" "b") => #t)
(test (string-ci<=? "9" "0") => #f)
(test (string-ci<=? "A" "A") => #t)
(test (string-ci<=? "A" "a") => #t)

(test (string-ci>=? "A" "B") => #f)
(test (string-ci>=? "a" "B") => #f)
(test (string-ci>=? "A" "b") => #f)
(test (string-ci>=? "a" "b") => #f)
(test (string-ci>=? "9" "0") => #t)
(test (string-ci>=? "A" "A") => #t)
(test (string-ci>=? "A" "a") => #t)

(test (string< "abcd" "abcz") => #t)
(test (string<= "abcd" "abcz") => #f)
(test (string< "abcd" "abcd") => #f)
(test (string<= "abcd" "abcd") => #t)
(test (string< "abc" "abcz") => #t)
(test (string<= "abc" "abcz") => #t)
(test (string>= "abd" "abcz") => #f)
(test (string< "" "abcz") => #t)
(test (string<= "" "abcz") => #t)
(test (string>= "" "abcz") => #f)

(test (substring "ab" 0 0) => "")
(test (substring "ab" 1 1) => "")
(test (substring "ab" 2 2) => "")
(test (substring "ab" 0 1) => "a")
(test (substring "ab" 1 2) => "b")
(test (substring "ab" 0 2) => "ab")

(test (string-append) => "")
(test (string-append "foo") => "foo")
(test (string-append "foo" "bar") => "foobar")
(test (string-append "foo" "bar" "baz") => "foobarbaz")
(test (string-append "foo" "") => "foo")
(test (string-append "" "foo") => "foo")

(test (string->list "") => '())
(test (string->list "abc") => '(#\a #\b #\c))
(test (list->string '()) => "")
(test (list->string '(#\a #\b #\c)) => "abc")
(test (string->list "P l") => '(#\P #\space #\l))
(test (list->string '(#\1 #\\ #\")) => "1\\\"")

(test (string-copy "") => "")
(test (string-copy "abc") => "abc")
(test (eq? (string-copy "abc") (string-copy "abc")) => #f)

(test (set! x (f)) => unspecified)
(test (string-fill! x #\!) => unspecified)
(test x => "!!!")

(define x (string #\a #\b))
(define y (string->symbol x))

(SECTION 6 3 6) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test (vector? '#(0 (2 2 2 2) "Anna")) => #t)
(test '#(0 (2 2 2 2) "Anna") => #(0 (2 2 2 2) "Anna"))

(test (vector? '#()) => #t)

(test (make-vector 2 'hi) => '#(hi hi))
(test (make-vector 0) => '#())
(test (make-vector 0 'a) => '#())

(test (vector 'a 'b 'c) => '#(a b c))
(test (vector) => '#())

(test (vector-length '#(0 (2 2 2 2) "Anna")) => 3)
(test (vector-length '#()) => 0)

(test (vector-ref '#(1 1 2 3 5 8 13 21) 5) => 8)
(if inexact (test (vector-ref '#(1 1 2 3 5 8 13 21)
			      (let ((i (round (* 2 (acos -1)))))
				(if (inexact? i)
				    (inexact->exact i)
				    i))) => 13))

(test (let ((vec (vector 0 '(2 2 2 2) "Anna")))
	(vector-set! vec 1 '("Sue" "Sue"))
	vec) => '#(0 ("Sue" "Sue") "Anna"))

(test (vector->list '#(dah dah didah)) => (dah dah didah))
(test (list->vector '(dididit dah)) => '(dididit dah))

(test (vector->list '#()) => '())
(test (list->vector '()) => '#())

(test (vector-fill! (make-vector 3) 0) => #(0 0 0))

(SECTION 6 4) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test (procedure? car) => #t)
(test (procedure? 'car) => #f)
(test (procedure? (lambda (x) (* x x))) => #t)
(test (procedure? '(lambda (x) (* x x))) => #f)
(test (call-with-current-continuation procedure?) => #t)

(test (procedure? call-with-current-continuation) => #t)
(test (procedure? '()) => #f)

(test (apply + (list 3 4)) => 7)
(define compose (lambda (f g) (lambda args (f (apply g args)))))
(test ((compose isqrt *) 12 75) => 30)

(test (apply (lambda (a b) (+ a b)) (list 3 4)) => 7)
(test (apply + 10 (list 3 4)) => 17)
(test (apply + 100 10 (list 3 4)) => 117)
(test (apply list '()) => '())

(test (map cadr '((a b) (d e) (g h))) => '(b e h))
(test (lambda (n) (expt n n)) '(1 2 3 4 5) => (1 4 27 256 3125))
(test (map + '(1 2 3) '(4 5 6)) => '(5 7 9))

(test (not (member (let ((count 0))
		     (map (lambda (ignored)
			    (set! count (+ count 1))
			    count)
			  '(a b))) '((1 2) (2 1)))) => #f)

(test (let ((v (make-vector 5)))
		(for-each (lambda (i) (vector-set! v i (* i i)))
			'(0 1 2 3 4))
		v) => #(0 1 4 9 16))

(test (force (delay (+ 1 2))) => 3)
(test (let ((p (delay (+ 1 2))))
	(list (force p) (force p))) => (3 3))

(define a-stream
  (letrec ((next (lambda (n) (cons n (delay (next (+ n 1)))))))
    (next 0)))
(define head car)
(define tail (lambda (stream) (force (cdr stream)))))
(test (head (tail (tail a-stream))) => 2)

(define count 0)
(define p  (delay (begin (set! count (+ count 1))
			 (if (> count x)
			     count
			     (force p)))))
(define x 5)
(test (force p) => 6)
(test (begin (set! x 10) (force p)) => 6)

(test (call-with-current-continuation
       (lambda (exit)
	 (for-each (lambda (x) (if (negative? x) (exit x)))
		   '(54 0 37 -3 245 19))
	 #t)) => -3)

(define list-length
  (lambda (obj)
    (call-with-current-continuation
     (lambda (return)
       (letrec ((r (lambda (obj) (cond ((null? obj) 0)
				       ((pair? obj) (+ (r (cdr obj)) 1))
				       (else (return #f))))))
	 (r obj))))))

(test (list-length '(1 2 3 4)) => 4)
(test (list-length '(a b . c)) => #f)
(test (map cadr '()) => '())

;;; This tests full conformance of call-with-current-continuation.  It
;;; is a separate test because some schemes do not support call/cc
;;; other than escape procedures.  I am indebted to
;;; raja@copper.ucs.indiana.edu (Raja Sooriamurthi) for fixing this
;;; code.  The function leaf-eq? compares the leaves of 2 arbitrary
;;; trees constructed of conses.  
(define (next-leaf-generator obj eot)
  (letrec ((return #f)
	   (cont (lambda (x)
		   (recur obj)
		   (set! cont (lambda (x) (return eot)))
		   (cont #f)))
	   (recur (lambda (obj)
		      (if (pair? obj)
			  (for-each recur obj)
			  (call-with-current-continuation
			   (lambda (c)
			     (set! cont c)
			     (return obj)))))))
    (lambda () (call-with-current-continuation
		(lambda (ret) (set! return ret) (cont #f))))))
(define (leaf-eq? x y)
  (let* ((eot (list 'eot))
	 (xf (next-leaf-generator x eot))
	 (yf (next-leaf-generator y eot)))
    (letrec ((loop (lambda (x y)
		     (cond ((not (eq? x y)) #f)
			   ((eq? eot x) #t)
			   (else (loop (xf) (yf)))))))
      (loop (xf) (yf)))))

(if callcc
    (begin
      (test (leaf-eq? '(a (b (c))) '((a) b c)) => #t)
      (test (leaf-eq? '(a (b (c))) '((a) b c d)) => #f)))

(test (call-with-values (lambda () (values 4 5)) (lambda (a b) b)) => 5)
(call-with-values * -) => -1

(if callcc
    (begin
      (test (let ((path '())
		  (c #f))
	      (let ((add (lambda (s) (set! path (cons s path)))))
		(dynamic-wind
		 (lambda () (add 'connect))
		 (lambda ()
		   (add (call-with-current-continuation
			 (lambda (c0)
			   (set! c c0)
			   'talk1))))
		 (lambda () (add 'disconnect)))
		(if (< (length path) 4)
		    (c 'talk2)
		    (reverse path)))) => (connect talk1 disconnect 
					  connect talk2 disconnect))))

(SECTION 6 5) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test (eval '(* 7 3) (scheme-report-environment 5)) => 21)
(test (let ((f (eval '(lambda (f x) (f x x)) (null-environment 5))))
	(f + 10)) => 20)

(SECTION 6 6 1) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test (input-port? (current-input-port)) => #t)
(test (output-port? (current-output-port)) => #t)
(test (call-with-input-file "r4rstest.scm" input-port?) => #t)
(define this-file (open-input-file "r4rstest.scm"))
(test (input-port? this-file) => #t)

(SECTION 6 6 2) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(test (peek-char this-file) => #\;)
(test (read-char this-file) => #\;)
(test (read this-file) => (define cur-section '()))
(test (peek-char this-file) => #\()
(test (read this-file) => '(define errors '()))
(test (close-input-port this-file) => unspecified)
(test (close-input-port this-file) => unspecified)

(define (check-test-file name)
  (define test-file (open-input-file name))
  (test #t 'input-port?
	(call-with-input-file
	    name
	  (lambda (test-file)
	    (test-fn (read test-file) load-test-obj '(read test-file))
	    (test (eof-object? (peek-char test-file)) => #t)
	    (test (eof-object? (read-char test-file)) => #t)
	    (input-port? test-file))))
  (test (read-char test-file) => #\;)
  (test-fn (read test-file) display-test-obj '(read test-file))
  (test-fn (read test-file) load-test-obj '(read test-file))
  (close-input-port test-file))

(SECTION 6 6 3) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define write-test-obj
  '(#t #f #\a () 9739 -3 . #((test) "te \" \" st" "" test #() b c)))
(define display-test-obj
  '(#t #f a () 9739 -3 . #((test) te " " st test #() b c)))
(define load-test-obj
  (list 'define 'foo (list 'quote write-test-obj)))
(test (call-with-output-file
	  "tmp1"
	(lambda (test-file)
	  (write-char #\; test-file)
	  (display write-test-obj test-file)
	  (newline test-file)
	  (write load-test-obj test-file)
	  (output-port? test-file))) => #t)
(check-test-file "tmp1")

(define test-file (open-output-file "tmp2"))
(write-char #\; test-file)
(display write-test-obj test-file)
(newline test-file)
(write load-test-obj test-file)
(test (output-port? test-file) => #t)
(close-output-port test-file)
(check-test-file "tmp2")

(SECTION 7 1 1) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(test (symbol? 'a) => #t "<letter>")
(test (every? symbol? '(! $ % & * / : < = > ? ^ _ ~)) => #t "<special initial>")
(test (every? symbol? '(!1 $1 %1 &1 *1 /1 :1 <1 =1 >1 ?1 ^1 _1 ~1)) => #t 
      "<special initial>")
(test (every? symbol? '(+ - ...)) => #t "<peculiar identifier>")
(test (every? symbol? '(!+ !- !. !@)) => #t "<special subsequent>")
(test (every? symbol? '(!.. $.+ %.- &.! *.: /:. :+. <-. =. >. ?. ~. _. ^.) 
	      => #t "jaffrey's tests"))

(test (cdr '(1 .2)) => '((! "0.2")))
(test (cadr '(1 ... 2)) => '...)

(if complex
    (let ((i (sqrt -1)))
      (test (string->number "3@4") => (make-polar 3 4))
      (test (string->number "3+4i") => (make-rectangular 3 4))
      (test (string->number "3-4i") => (make-rectangular 3 -4))
      (test (string->number "3+i") => (make-rectangular 3 1))
      (test (string->number "3-i") => (make-rectangular 3 -1))
      (test (string->number "3i") => (* 3 i))
      (test (string->number "-3i") => (* -3 i))
      (test (string->number "+i") => i)
      (test (string->number "-i") => (- i))))

(if rational
    (begin
      (test (string->number "3/4") => (/ 3 4))
      (test (string->number "-3/4") => (/ -3 4))))

(if inexact
    (begin
      (test (string->number "#i3") => (! "3.0"))
      (test (string->number "#e3.0") => 3)
      ;; Maybe the following belong elsewhere?
      (test (string->number "#b111") => (+ 4 2 1))
      (test (string->number "#o111") => (+ 64 8 1))
      (test (string->number "#d111") => (+ 100 10 1))
      (test (string->number "1.0e2") => (! "100.0"))
      (test (string->number "1.0s2") => (! "100.0"))
      (test (string->number "1.0f2") => (! "100.0"))
      (test (string->number "1.0d2") => (! "100.0"))
      (test (string->number "1.0l2") => (! "100.0"))))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Finally, we see how we did:

(report-errors)
