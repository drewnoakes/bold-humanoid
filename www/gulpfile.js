/// <reference path="scripts/libs/gulp.d.ts" />

// TODO investigate gulp-type for incremental compiles https://www.npmjs.org/package/gulp-type/
// TODO investigate using browserify or webpack and AMD modules
// TODO investigate keeping sourcemaps through pipeline
//      - code: https://github.com/gulpjs/gulp/blob/master/docs/recipes/browserify-uglify-sourcemap.md
//      - styles: https://www.npmjs.org/package/gulp-autoprefixer
// TODO investigate running tests using gulp

var gulp = require('gulp');

var sass = require('gulp-sass');
var concat = require('gulp-concat');
var uglify = require('gulp-uglify');
var rename = require('gulp-rename');
var autoprefixer = require('gulp-autoprefixer');
var minifycss = require('gulp-minify-css');
var minifyhtml = require('gulp-minify-html');
var fs = require('fs');
var header = require('gulp-header');
var typescript = require('gulp-tsc');
var amdOptimize = require('amd-optimize');

var outFolder = 'dist';

gulp.task('styles', function ()
{
    return gulp.src('styles/*.scss')
        .pipe(sass())
        .pipe(autoprefixer('last 2 versions', 'safari 5', 'ie 8', 'ie 9', 'opera 12.1'))
        .pipe(gulp.dest('styles'))
        .pipe(rename({suffix: '.min'}))
        .pipe(minifycss())
        .pipe(header(fs.readFileSync('LICENSE')))
        .pipe(gulp.dest(outFolder));
});

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

gulp.task('html', function ()
{
    return gulp.src('index.html')
        .pipe(minifyhtml())
        .pipe(gulp.dest(outFolder));
});

gulp.task('watch', function ()
{
    gulp.watch('scripts/app/**/*.ts', ['scripts']);
    gulp.watch('styles/*.scss', ['styles']);
});

gulp.task('default', ['styles', 'scripts']);
