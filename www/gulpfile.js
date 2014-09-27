/// <reference path="scripts/libs/gulp.d.ts" />

var gulp = require('gulp');

var concat = require('gulp-concat');
var uglify = require('gulp-uglify');
var rename = require('gulp-rename');
var fs = require('fs');
var header = require('gulp-header');
var typescript = require('gulp-tsc');
var amdOptimize = require('amd-optimize');

var outFolder = 'dist';

gulp.task('tsc', function ()
{
    return gulp.src('scripts/app/**/*.ts')
        .pipe(typescript({module:'amd'}))
        .pipe(gulp.dest('scripts/app/'))
});

gulp.task('scripts', ['tsc'], function ()
{
    return gulp.src('scripts/app/**/*.js')
        .pipe(amdOptimize('main', {baseUrl:'scripts/app'}))
        .pipe(concat('main.js'))
        .pipe(gulp.dest(outFolder))
        .pipe(rename('main.min.js'))
        .pipe(uglify())
        .pipe(header(fs.readFileSync('LICENSE')))
        .pipe(gulp.dest(outFolder));
});
