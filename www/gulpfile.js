/// <reference path="scripts/libs/gulp.d.ts" />

var gulp = require('gulp');

var typescript = require('gulp-tsc');

gulp.task('tsc', function ()
{
    return gulp.src('scripts/app/**/*.ts')
        .pipe(typescript({module:'amd'}))
        .pipe(gulp.dest('scripts/app/'))
});
